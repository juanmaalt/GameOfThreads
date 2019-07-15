/*
 * Sistema_de_criterios.c
 *
 *  Created on: 20 jun. 2019
 *      Author: facusalerno
 */

#include "Sistema_de_criterios.h"

//FUNCIONES: Privadas: buscar
static bool memoria_esta_en_la_lista(t_list *lista, int numeroMemoria);

//FUNCIONES: Privadas: buscar y traer
static MetadataTabla *machearTabla(char *tabla);//Dado el nombre de una tabla devuelve la estructura que la representa. Si no existe devuelve null
static Memoria *machearMemoria(int numeroMemoria);

//FUNCIONES: Privadas: asignacion de memorias
static Memoria *sc_determinar_memoria(MetadataTabla *tabla); //Dada una tabla con SC, determina que memoria la deberia atender
static Memoria *hsc_determinar_memoria(MetadataTabla *tabla, char *key);
static Memoria *ec_determinar_memoria(MetadataTabla *tabla);

//FUNCIONES: Privadas: gestion de tablas
static MetadataTabla *crear_tabla(char* nombre, char *consistencia, char *particiones, char *tiempoEntreCompactacion);
static Memoria *crear_memoria(int numero, char *ip, char *puerto);
static void remover_referencia_tabla(MetadataTabla *tabla);



int iniciar_sistema_de_criterios(void){
	memoriasSC = list_create();
	memoriasHSC = list_create();
	memoriasEC = list_create();
	memoriasExistentes = list_create();
	tablasExistentes = list_create();
	return EXIT_SUCCESS;
}





Memoria *determinar_memoria_para_tabla(char *nombreTabla, char *keyDeSerNecesaria){ //No liberar nunca nombreTabla ni key por que es: o parte de una linea lql o parte de una request individual. Lo que se tenga que liberar, se libera en Unidad_de_ejecucion.c
	MetadataTabla *tabla;
	if(nombreTabla == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: determinar_memoria_para_tabla: no se especifico el nombre de la tabla");
		log_info(logger_invisible, "Sistema_de_criterios.c: determinar_memoria_para_tabla: no se especifico el nombre de la tabla");
		return NULL;
	}

	if((tabla = machearTabla(nombreTabla)) == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: determinar_memoria_para_tabla: la tabla no existe o aun no se conoce");
		log_info(logger_invisible, "Sistema_de_criterios.c: determinar_memoria_para_tabla: la tabla no existe o aun no se conoce");
		return NULL;
	}

	switch(tabla->consistencia){
	case SC:
		return sc_determinar_memoria(tabla);
	case SHC:
		return hsc_determinar_memoria(tabla, keyDeSerNecesaria);
	case EC:
		return ec_determinar_memoria(tabla);
	default:
		return NULL;
	}
	return NULL;
}





Memoria *elegir_cualquiera_ec(){
	Memoria *m = (Memoria*)list_get(memoriasEC, getNumberUntil(list_size(memoriasEC)));
	if(m == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: elegir_cualquiera: no hay memorias EC para responder a la request casual");
		log_info(logger_invisible, "Sistema_de_criterios.c: elegir_cualquiera: no hay memorias EC para responder a la request casual");
	}
	return m;
}





int asociar_memoria(char *numeroMemoria, char *consistencia){
	Memoria *memoria;
	if((memoria = machearMemoria(atoi(numeroMemoria))) == NULL)
		RETURN_ERROR("Sistema_de_criterios.c: add_memory: el numero de memoria es invalido o aun no se conoce");

	if(string_equals_ignore_case(consistencia, "SC")){
		if(!memoria_esta_en_la_lista(memoriasSC, memoria->numero)){
			memoria->Metrics.SC.estaAsociada = true;
			list_add(memoriasSC, memoria);
		}else{
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			log_info(logger_invisible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			return EXIT_SUCCESS;
		}
	}
	else if(string_equals_ignore_case(consistencia, "SHC")){
		if(!memoria_esta_en_la_lista(memoriasHSC, memoria->numero)){
			memoria->Metrics.SHC.estaAsociada = true;
			list_add(memoriasHSC, memoria);
		}else{
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			log_info(logger_invisible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			return EXIT_SUCCESS;
		}
	}
	else if(string_equals_ignore_case(consistencia, "EC")){
		if(!memoria_esta_en_la_lista(memoriasEC, memoria->numero)){
			memoria->Metrics.EC.estaAsociada = true;
			list_add(memoriasEC, memoria);
		}else{
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			log_info(logger_invisible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			return EXIT_SUCCESS;
		}
	}else RETURN_ERROR("Sistema_de_criterios.c: add_memory: el tipo de consistencia es invalido. Solo se admite SC, SHC o EC");
	memoria->fueAsociada = true;
	log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria fue correctamente asignada");
	log_info(logger_invisible, "Sistema_de_criterios.c: add_memory: la memoria fue correctamente asignada");
	return EXIT_SUCCESS;
}





int procesar_describe_global(char *cadenaResultadoDescribe){
	void destruir_tabla_encontrada(void *tabla){
		free(((MetadataTabla*)tabla)->nombre);
		free((MetadataTabla*)tabla);
	}
	if(cadenaResultadoDescribe == NULL){
		list_destroy_and_destroy_elements(tablasExistentes, destruir_tabla_encontrada);
		tablasExistentes = list_create();
		return EXIT_SUCCESS;
	}
	if(tablasExistentes == NULL)
		return EXIT_FAILURE;

	MetadataTabla *tabla;
	t_list *listaAuxiliar = list_create(); //Creo una lista vacia sobre la cual voy a trabajar

	char **descompresion = descomprimir_describe(cadenaResultadoDescribe); //Descomprimo los resultados del ultimo describe
	for(int i=0; descompresion[i]!= NULL; i+=4){ //Este for maneja los bloques de la descompresion como 4-upla ya que la cadena esta convenientemente ordenada
		if((tabla = machearTabla(descompresion[i])) == NULL){ //Buscar la tabla (por nombre) en la lista de tablasExistentes
			tabla = crear_tabla(descompresion[i], descompresion[i+1], descompresion[i+2], descompresion[i+2]); //Si no se encuentra, creo una nueva
			if(tabla == NULL) return EXIT_FAILURE; //El crear tabla puede fallar
			list_add(listaAuxiliar, tabla); //La agrego a mi lista auxiliar
		}else{
			remover_referencia_tabla(tabla); //Remuevo la referencia de la lista de tablasExistentes para que me quede solo con las tablas que NO estaban en la cadena de describe que se proceso
			list_add(listaAuxiliar, tabla); //Si se encuentra, agrego esa referencia de tablasExistentes a mi listaAuxiliar. Esto lo hago asi por que hay ciertos atributos que se asignan en momentos distintos, por ejemplo, a una tabla SC se le asigna una memoria, y necesito que siga siendo siempre la misma, por eso quiero usar la misma referencia a esa tabla.
		}
	}
	destruir_split_tablas(descompresion);
	list_destroy_and_destroy_elements(tablasExistentes, destruir_tabla_encontrada); //Libero las referencias de la lista que quedaron, que fueron los que se dieron de baja (indirectamente, gracias al remover referencia que removio las que si estaban)
	tablasExistentes = list_duplicate(listaAuxiliar); //Duplico la lista auxiliar con todos los elementos del nuevo describe, manteniendo los del anterior describe (son sus respecrtivos atributos de criterios), y eliminando los viejos (ya que nunca se agregaron a la listaAuxiliar)
	list_destroy(listaAuxiliar);
	return EXIT_SUCCESS;
}





int procesar_describe_simple(char *cadenaResultadoDescribe, char *instruccionActual){
	if(cadenaResultadoDescribe == NULL){
		Comando comando = parsear_comando(instruccionActual);
		remover_metadata_tabla(machearTabla(comando.argumentos.DESCRIBE.nombreTabla));
		destruir_comando(comando);
		return EXIT_SUCCESS;
	}

	MetadataTabla *tabla;

	char **descompresion = descomprimir_describe(cadenaResultadoDescribe);
	for(int i=0; descompresion[i]!= NULL; i+=4){
		if((tabla = machearTabla(descompresion[i])) == NULL){
			tabla = crear_tabla(descompresion[i], descompresion[i+1], descompresion[i+2], descompresion[i+2]);
			if(tabla == NULL) return EXIT_FAILURE;
		}
		//En este caso si la tabla se encuentra no hago nada. Para mejorar la robustez podria hacer que el machear tabla busque coincidencias por mas atributos de la tabla y no solo por el nombre
	}

	return EXIT_SUCCESS;
}





void mostrar_describe(char *cadenaResultadoDescribe){
	int usarCadenaResultado = FALSE;
	if(usarCadenaResultado)
		goto CAD; //Muestra el contenido de la cadena, que podria no ser el de la lista de tablas existentes, en un momento dado
	else
		goto LIST; //Muestra el contenido real de la lista de tablas existentes

	CAD: ;
	char **descompresion = descomprimir_describe(cadenaResultadoDescribe);
	for(int i=0; descompresion[i]!= NULL; i+=4){
		printf(GRN"Tabla: %s | "STD, descompresion[i]);
		printf("Consistencia: %s | ", descompresion[i+1]);
		printf("Numero de particiones: %s | ", descompresion[i+2]);
		printf("Tiempo entre compactacion: %s", descompresion[i+3]);
		printf("\n");
	}
	destruir_split_tablas(descompresion);
	return ;

	LIST: ;
	if(list_is_empty(tablasExistentes)){
		printf(BLU"No se encontraron metadatas\n"STD);
		return;
	}
	void mostrar(void *elemento){
		printf(GRN"Tabla: %s | "STD, ((MetadataTabla*)elemento)->nombre);
		printf("Consistencia: %d | ", ((MetadataTabla*)elemento)->consistencia);
		printf("Numero de particiones: %d | ", ((MetadataTabla*)elemento)->particiones);
		printf("Tiempo entre compactacion: %d\n", ((MetadataTabla*)elemento)->tiempoEntreCompactaciones);
	}
	printf(BLU"0: SC | 1: SHC | 2: EC\n"STD);
	list_iterate(tablasExistentes, mostrar);
	return ;
}





t_list *procesar_gossiping(char *cadenaResultadoGossiping){
	Memoria *memoria;
	t_list *lista = list_create();

	char **descompresion = descomprimir_memoria(cadenaResultadoGossiping);
	for(int i=0; descompresion[i]!=NULL; i+=3){
		int socket;
		if((socket = connect_to_server(descompresion[i+1], descompresion[i+2])) == EXIT_FAILURE){
			continue;
		}
		close(socket);
		memoria = crear_memoria(atoi(descompresion[i]), descompresion[i+1], descompresion[i+2]);
		list_add(lista, memoria);
	}
	destruir_split_memorias(descompresion);
	return lista;
}





Consistencia consistencia_de_tabla(char *nombreTabla){
	MetadataTabla *tabla = machearTabla(nombreTabla);
	if(tabla == NULL)
		return -1;
	else
		return tabla->consistencia;
}





void remover_memoria(Memoria *memoria_a_remover){
	bool remover_por_numero(void *memoria){
		return memoria_a_remover->numero == ((Memoria*)memoria)->numero;
	}
	void destruir_memoria_encontrada(void *memoria){
		free(((Memoria*)memoria)->ip);
		free(((Memoria*)memoria)->puerto);
		free((Memoria*)memoria);
	}
	list_remove_by_condition(memoriasEC, remover_por_numero); //Solo remueven los nodos
	list_remove_by_condition(memoriasHSC, remover_por_numero);
	list_remove_by_condition(memoriasSC, remover_por_numero);
	list_remove_and_destroy_by_condition(memoriasExistentes, remover_por_numero, destruir_memoria_encontrada); //Y el ultimo que remueva el nodo tambien deberia liberarlo
}





void remover_metadata_tabla(MetadataTabla *tabla){
	bool remover_por_nombre(void *elemento){
		return !strcpy(((MetadataTabla*)elemento)->nombre, tabla->nombre);
	}
	void destruir_tabla_encontrada(void *tabla){
		free(((MetadataTabla*)tabla)->nombre);
		free((MetadataTabla*)tabla);
	}
	list_remove_and_destroy_by_condition(tablasExistentes, remover_por_nombre, destruir_tabla_encontrada);
}





void agregar_sin_repetidos(t_list *destino, t_list *fuente){
	void agregar_distinct(void *elementoFuente){
		bool buscar(void *elementoDestino){
			return ((Memoria*)elementoDestino)->numero == ((Memoria*)elementoFuente)->numero;
		}
		if(!list_any_satisfy(destino, buscar)){
			list_add(destino, elementoFuente);
		}
	}
	list_iterate(fuente, agregar_distinct);
}





void agregar_metadata_tabla(char *nombre, char *consistencia, char *particiones, char *tiempoEntreCompactacion){
	MetadataTabla *tabla = crear_tabla(nombre, consistencia, particiones, tiempoEntreCompactacion);
	if(tabla == NULL){
		log_error(logger_visible, "Sistema_de_criterios.c: agregar_metadata_tabla: no se puede agregar la metadata de la tabla %s", nombre);
		log_error(logger_invisible, "Sistema_de_criterios.c: agregar_metadata_tabla: no se puede agregar la metadata de la tabla %s", nombre);
		return;
	}
	list_add(tablasExistentes, tabla);
}





bool tabla_esta_en_la_lista(char *tabla){
	if(tablasExistentes == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: tabla_esta_en_la_lista: aun no existen tablas conocidas en el sistema");
		log_info(logger_invisible, "Sistema_de_criterios.c: tabla_esta_en_la_lista: aun no existen tablas conocidas en el sistema");
		return NULL;
	}
	bool buscar(void *elemento){
		return !strcmp(tabla, ((MetadataTabla*)elemento)->nombre); //Devuelve 0 (falso) si son iguales
	}
	return list_any_satisfy(tablasExistentes, buscar);
}





static bool memoria_esta_en_la_lista(t_list *lista, int numeroMemoria){
	if(lista == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: memoria_esta_en_la_lista: aun no existen memoria conocidas en el sistema");
		log_info(logger_invisible, "Sistema_de_criterios.c: memoria_esta_en_la_lista: aun no existen memoria conocidas en el sistema");
		return NULL;
	}
	bool buscar(void *elemento){
		return ((Memoria*)elemento)->numero == numeroMemoria;
	}
	return list_any_satisfy(lista, buscar);
}





static MetadataTabla *machearTabla(char *tabla){
	if(tablasExistentes == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: machearTabla: aun no existen tablas conocidas en el sistema");
		log_info(logger_invisible, "Sistema_de_criterios.c: machearTabla: aun no existen tablas conocidas en el sistema");
		return NULL;
	}
	bool buscar(void *elemento){
		return !(strcmp(tabla, ((MetadataTabla*)elemento)->nombre) * strcmp(tabla, ((MetadataTabla*)elemento)->nombre)); //Devuelve 0 (falso) si son iguales. Si es negativo lo multiplica por sis mismo para ser posiitiivoooollk
	}
	return (MetadataTabla*)list_find(tablasExistentes, buscar);
}





static Memoria *machearMemoria(int numeroMemoria){
	if(memoriasExistentes == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: machearMemoria: aun no existen memorias conocidas en el sistema");
		log_info(logger_invisible, "Sistema_de_criterios.c: machearMemoria: aun no existen memorias conocidas en el sistema");
		return NULL;
	}
	bool buscar(void *elemento){
		return numeroMemoria == ((Memoria*)elemento)->numero;
	}
	return (Memoria*)list_find(memoriasExistentes, buscar);
}





static Memoria *sc_determinar_memoria(MetadataTabla *tabla){
	if(memoriasSC == NULL) //Si la lista es NULL significa que nunca se creo, por lo tanto asumo que trabajamos solo com memoria principal. Si la lista es vacia ese es otro tema
		return NULL;

	if(tabla->Atributos.SC.memoriaAsignada == NULL){
		if(!list_is_empty(memoriasSC)){
			tabla->Atributos.SC.memoriaAsignada = list_get(memoriasSC, getNumberUntil(list_size(memoriasSC))); //Si no tiene memoria asignada se le asigna una random que va a ser permanente
		}else{
			log_error(logger_error, "Sistema_de_criterios.c: sc_determinar_memoria: No se puede responder la request por que no hay memorias Strong Consistency disponibles");
			log_info(logger_invisible, "Sistema_de_criterios.c: sc_determinar_memoria: No se puede responder la request por que no hay memorias Strong Consistency disponibles");
			return NULL;
		}
	}
	return tabla->Atributos.SC.memoriaAsignada;
}





static Memoria *hsc_determinar_memoria(MetadataTabla *tabla, char *key){ //La verdad que la tabla no se usa para nada pero bueno paja
	if(memoriasHSC == NULL)
		return NULL;
	if(list_is_empty(memoriasHSC)){
		log_error(logger_error, "Sistema_de_criterios.c: hsc_determinar_memoria: No se puede responder la request por que no hay memorias Hash Strong Consistency disponibles");
		log_info(logger_invisible, "Sistema_de_criterios.c: hsc_determinar_memoria: No se puede responder la request por que no hay memorias Hash Strong Consistency disponibles");
		return NULL;
	}
	return key == NULL ? (Memoria*)list_get(memoriasHSC, 0) : (Memoria*)list_get(memoriasHSC, getHash(key, list_size(memoriasHSC)));
}





static Memoria *ec_determinar_memoria(MetadataTabla *tabla){
	if(memoriasEC == NULL)
		return NULL;
	if(list_is_empty(memoriasEC)){
		log_error(logger_error, "Sistema_de_criterios.c: ec_determinar_memoria: No se puede responder la request por que no hay memorias Eventual Consistency disponibles");
		log_info(logger_invisible, "Sistema_de_criterios.c: ec_determinar_memoria: No se puede responder la request por que no hay memorias Eventual Consistency disponibles");
		return NULL;
	}
	return (Memoria*)list_get(memoriasEC, getNumberUntil(list_size(memoriasEC)));
}





static MetadataTabla *crear_tabla(char* nombre, char *consistencia, char *particiones, char *tiempoEntreCompactacion){
	MetadataTabla *retorno = malloc(sizeof(MetadataTabla));
	retorno->nombre = string_from_format(nombre);
	if(string_equals_ignore_case(consistencia, "SC")){
		retorno->consistencia = SC;
		retorno->Atributos.SC.memoriaAsignada = NULL;
	}else if(string_equals_ignore_case(consistencia, "SHC")){
		retorno->consistencia = SHC;
	}else if(string_equals_ignore_case(consistencia, "EC")){
		retorno->consistencia = EC;
	}else return NULL;
	retorno->particiones = atoi(particiones);
	retorno->tiempoEntreCompactaciones = atoi(tiempoEntreCompactacion);
	return retorno;
}





static void remover_referencia_tabla(MetadataTabla *tabla){
	bool remover_por_nombre(void *elemento){
		return !strcmp(((MetadataTabla *)elemento)->nombre, tabla->nombre);
	}
	list_remove_by_condition(tablasExistentes, remover_por_nombre);
}





static Memoria *crear_memoria(int numero, char *ip, char *puerto){
	Memoria *memoria = malloc(sizeof(Memoria));
	memoria->numero = numero;
	memoria->ip = string_from_format(ip);
	memoria->puerto = string_from_format(puerto);
	memoria->fueAsociada = false;
	memoria->Metrics.EC.estaAsociada = false;
	memoria->Metrics.EC.cantidadInsert = 0;
	memoria->Metrics.EC.cantidadSelect = 0;
	memoria->Metrics.SC.estaAsociada = false;
	memoria->Metrics.SC.cantidadInsert = 0;
	memoria->Metrics.SC.cantidadSelect = 0;
	memoria->Metrics.SHC.estaAsociada = false;
	memoria->Metrics.SHC.cantidadInsert = 0;
	memoria->Metrics.SHC.cantidadSelect = 0;
	return memoria;
}

