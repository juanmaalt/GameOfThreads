/*
 * Sistema_de_criterios.c
 *
 *  Created on: 20 jun. 2019
 *      Author: facusalerno
 */

#include "Sistema_de_criterios.h"

//FUNCIONES: Privadas
static bool tabla_esta_en_la_lista(char *tabla);
static bool memoria_esta_en_la_lista(t_list *lista, int numeroMemoria);
static MetadataTabla *machearTabla(char *tabla);//Dado el nombre de una tabla devuelve la estructura que la representa. Si no existe devuelve null
static Memoria *machearMemoria(int numeroMemoria);
static Memoria *sc_determinar_memoria(MetadataTabla *tabla); //Dada una tabla con SC, determina que memoria la deberia atender
static Memoria *hsc_determinar_memoria(MetadataTabla *tabla);
static Memoria *ec_determinar_memoria(MetadataTabla *tabla);





int iniciar_sistema_de_criterios(void){
	memoriasSC = list_create();
	memoriasHSC = list_create();
	memoriasEC = list_create();
	memoriasExistentes = list_create();
	tablasExistentes = list_create();
	return EXIT_SUCCESS;
}





Memoria *determinar_memoria_para_tabla(char *nombreTabla){ //No liberar nunca nombreTabla por que es: o parte de una linea lql o parte de una request individual. Lo que se tenga que liberar, se libera en Unidad_de_ejecucion.c
	MetadataTabla *tabla;
	if((tabla = machearTabla(nombreTabla)) == NULL){
		log_error(logger_error, "Sistema_de_criterios.c: determinar_memoria_para_tabla: la tabla no existe o aun no se conoce");
		log_info(logger_invisible, "Sistema_de_criterios.c: determinar_memoria_para_tabla: la tabla no existe o aun no se conoce");
		return NULL;
	}

	switch(tabla->consistencia){
	case SC:
		return sc_determinar_memoria(tabla);
	case HSC:
		return hsc_determinar_memoria(tabla);
	case EC:
		return ec_determinar_memoria(tabla);
	default:
		return NULL;
	}
	return NULL;
}





int add_memory(char *numeroMemoria, char *consistencia){
	Memoria *memoria;
	if((memoria = machearMemoria(atoi(numeroMemoria))) == NULL)
		RETURN_ERROR("Sistema_de_criterios.c: add_memory: el numero de memoria es invalido o aun no se conoce");

	if(string_equals_ignore_case(consistencia, "SC")){
		if(!memoria_esta_en_la_lista(memoriasSC, memoria->numero)){
			list_add(memoriasSC, memoria);
		}else{
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			return EXIT_SUCCESS;
		}
	}
	if(string_equals_ignore_case(consistencia, "HSC")){
		if(!memoria_esta_en_la_lista(memoriasHSC, memoria->numero)){
			list_add(memoriasHSC, memoria);
		}else{
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			return EXIT_SUCCESS;
		}
	}
	if(string_equals_ignore_case(consistencia, "EC")){
		if(!memoria_esta_en_la_lista(memoriasEC, memoria->numero)){
			list_add(memoriasEC, memoria);
		}else{
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			log_info(logger_visible, "Sistema_de_criterios.c: add_memory: la memoria ya esta asignada al criterio");
			return EXIT_SUCCESS;
		}
	}else RETURN_ERROR("Sistema_de_criterios.c: add_memory: el tipo de consistencia es invalido. Solo se admite SC, HSC o EC");
	return EXIT_SUCCESS;
}





int procesar_describe(char *cadenaResultadoDescribe){
	if(tablasExistentes == NULL)
		return EXIT_FAILURE;

	char **descompresion = descomprimir_describe(cadenaResultadoDescribe);
	for(int i=0; descompresion[i]!= NULL; i+=4){
		MetadataTabla *tabla;
		if((tabla = machearTabla(descompresion[i])) == NULL){//Si ya existe, la trae de la lista y se pisan los valores con el objetivo de actualizarlos, pero mantener aquellos valores que no cambiaron. Por ejemplo, el caso de una tabla SC, su memoria asignada no cambia
			tabla = malloc(sizeof(MetadataTabla));//Si no existe, se crea una nueva
			list_add(tablasExistentes, tabla);
		}
		tabla->nombre = string_from_format(descompresion[i]);
		if(string_equals_ignore_case(descompresion[i+1], "SC"))
			tabla->consistencia = SC;
		else if(string_equals_ignore_case(descompresion[i+1], "HSC"))
			tabla->consistencia = HSC;
		else if(string_equals_ignore_case(descompresion[i+1], "EC"))
			tabla->consistencia = EC;
		else return EXIT_FAILURE;
		tabla->particiones = atoi(descompresion[i+2]);
		tabla->tiempoEntreCompactaciones = atoi(descompresion[i+3]);
	}
	destruir_split_tablas(descompresion);
	return EXIT_SUCCESS;
}





void mostrar_describe(char *cadenaResultadoDescribe){
	int usarCadena = FALSE; //Solo para debugear, muestra el contenido de la lista de tablas, o el contenido del ultimo resultado del describe (que seria la cadena)
	if(usarCadena)
		goto CAD;
	else
		goto LIST;

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
	void mostrar(void *elemento){
		printf(GRN"Tabla: %s | "STD, ((MetadataTabla*)elemento)->nombre);
		printf("Consistencia: %d | ", ((MetadataTabla*)elemento)->consistencia);
		printf("Numero de particiones: %d | ", ((MetadataTabla*)elemento)->particiones);
		printf("Tiempo entre compactacion: %d\n", ((MetadataTabla*)elemento)->tiempoEntreCompactaciones);
	}
	printf(BLU"0: SC | 1: HSC | 2: EC\n"STD);
	list_iterate(tablasExistentes, mostrar);
	return ;
}





int procesar_gossiping(char *cadenaResultadoGossiping){
	if(memoriasExistentes == NULL)
		return EXIT_FAILURE;

	char **descompresion = descomprimir_memoria(cadenaResultadoGossiping);
	for(int i=0; descompresion[i]!=NULL; i+=3){
		Memoria *memoria;
		if((memoria = machearMemoria(atoi(descompresion[i]))) == NULL){
			Memoria *memoria = malloc(sizeof(Memoria));
			list_add(memoriasExistentes, memoria);
		}
		memoria->numero = atoi(descompresion[i]);
		memoria->ip = descompresion[i+1];
		memoria->puerto = descompresion[i+2];
	}
	destruir_split_memorias(descompresion);
	return EXIT_SUCCESS;
}





static bool tabla_esta_en_la_lista(char *tabla){
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
		return !strcmp(tabla, ((MetadataTabla*)elemento)->nombre); //Devuelve 0 (falso) si son iguales
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





static Memoria *hsc_determinar_memoria(MetadataTabla *tabla){
	if(memoriasHSC == NULL)
		return NULL;
	if(list_is_empty(memoriasHSC)){
		log_error(logger_error, "Sistema_de_criterios.c: hsc_determinar_memoria: No se puede responder la request por que no hay memorias Hasg Strong Consistency disponibles");
		log_info(logger_invisible, "Sistema_de_criterios.c: hsc_determinar_memoria: No se puede responder la request por que no hay memorias Hash Strong Consistency disponibles");
		return NULL;
	}
	return (Memoria*)list_get(memoriasHSC, getHash(tabla->nombre, list_size(memoriasHSC)));
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

