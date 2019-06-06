/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Memoria
 ============================================================================
 */

#include "Memoria.h"

void mostrarPathSegmentos() {
	segmento_t * segmentoAux;

	for (int i = 0; i < list_size(tablaSegmentos.listaSegmentos); ++i) {
		segmentoAux = (segmento_t *) list_get(tablaSegmentos.listaSegmentos, i);
		printf("EL PATH DEL SEGMENTO ES: %s\n", obtenerPath(segmentoAux));

	}

}

//TODO: cada printf en rojo que indique el error se debe poner en el log

int main(void) {
	//Se hacen las configuraciones iniciales para log y config
	if (configuracion_inicial() == EXIT_FAILURE) {
		log_error(logger_invisible, "Memoria.c: main: no se pudo generar la configuracion inicial");

		return EXIT_FAILURE;
	}
	mostrar_por_pantalla_config();
	/*
	 if(realizarHandshake()==EXIT_FAILURE){
	 printf(RED"Memoria.c: main: no se pudo inicializar la memoria principal"STD"\n");
	 return EXIT_FAILURE;
	 }
	 */
	tamanioValue = 4;

	pathLFS = malloc(
			strlen("/puntoDeMontajeQueMeDaJuanEnElHandshake/") * sizeof(char)
					+ 1);
	strcpy(pathLFS, "/puntoDeMontajeQueMeDaJuanEnElHandshake/");

	// Inicializar la memoria principal
	if (inicializar_memoriaPrincipal() == EXIT_FAILURE) {
		log_error(logger_invisible, "Memoria.c: main: no se pudo inicializar la memoria principal");

		return EXIT_FAILURE;
	}
	printf("Memoria Inicializada correctamente\n");

	//TODO:GOSSIPING
	iniciar_gossiping();

	//FUNCIONES PARA TEST DE SELECT
	memoriaConUnSegmentoYUnaPagina();

	mostrarContenidoMemoria();
	mostrarPathSegmentos();

	//Inicio consola

	if (iniciar_consola() == EXIT_FAILURE) {
		log_error(logger_invisible, "Memoria.c: main: no se pudo levantar la consola");

		return EXIT_FAILURE;
	}

	pthread_join(idConsola, NULL); //detach cuando se agregue threadConnection

	//Habilita el server y queda en modo en listen
	/*
	 int miSocket = enable_server(fconfig.ip, fconfig.puerto);
	 log_info(logger_invisible, "Servidor encendido, esperando conexiones");
	 threadConnection(miSocket, connection_handler);
	 */

	//TODO: liberar todo
	if (memoriaPrincipal.memoria != NULL)
		free(memoriaPrincipal.memoria);
	config_destroy(configFile);
	log_destroy(logger_invisible);
	log_destroy(logger_visible);

	//free en memoriaPrincipal
	/*
	 *
	 *
	 queue_clean(memoriaPrincipal.marcosLibres);

	 list_destroy_and_destroy_elements(memoriaPrincipal.listaAdminMarcos, free());

	 */
}

int realizarHandshake(void) {
	int lfsSocket = conectarLFS();
	tamanioValue = handshakeLFS(lfsSocket);
	printf("TAMAÑO_VALUE= %d\n", tamanioValue);
	return EXIT_SUCCESS;
}

int handshakeLFS(int socketLFS) {
	send_msg(socketLFS, TEXTO_PLANO, "handshake");

	TipoDeMensaje tipo;
	char *tamanio = recv_msg(socketLFS, &tipo);

	if (tipo == COMANDO)
		printf("Handshake falló. No se recibió el tamaño del value.\n");
	if (tipo == TEXTO_PLANO)
		printf("Handshake exitoso. Se recibió el tamaño del value, es: %d\n",
				*tamanio);

	return *tamanio;
}

int conectarLFS() {
	//Obtiene el socket por el cual se va a conectar al LFS como cliente / * Conectarse al proceso File System
	int socket = connect_to_server(fconfig.ip_fileSystem,
			fconfig.puerto_fileSystem);
	if (socket == EXIT_FAILURE) {
		log_error(logger_invisible,
				"El LFS no está levantado. Cerrar la Memoria, levantar el LFS y volver a levantar la Memoria");
		return EXIT_FAILURE;
	}
	log_error(logger_invisible, "Conectado al LFS. Iniciando Handshake.");

	return socket;
}

char* obtenerPath(segmento_t* segmento) {
	return segmento->pathTabla;
}

void mostrarContenidoMemoria() {
	timestamp_t timestamp;
	uint16_t key;
	char* value = malloc (sizeof(char)*tamanioValue);
	memcpy(&timestamp,memoriaPrincipal.memoria,sizeof(timestamp_t));
	memcpy(&key,memoriaPrincipal.memoria+sizeof(timestamp_t),sizeof(uint16_t));
	strcpy(value,memoriaPrincipal.memoria+sizeof(timestamp_t)+ sizeof(uint16_t) );

	printf("Timestamp: %llu\nKey:%d\nValue: %s\n",timestamp,key,value);

	free(value);

}

void asignarPathASegmento(segmento_t * segmentoANombrar, char* nombreTabla) {
	segmentoANombrar->pathTabla = malloc(
			sizeof(char) * (strlen(pathLFS) + strlen(nombreTabla)) + 1);
	strcpy(segmentoANombrar->pathTabla, pathLFS);
	strcat(segmentoANombrar->pathTabla, nombreTabla);
}

void crearRegistroEnTabla(tabla_de_paginas_t *tablaDondeSeEncuentra, int indiceMarco) {
	registroTablaPag_t *registroACrear = malloc(sizeof(registroTablaPag_t));

	registroACrear->numeroPagina = indiceMarco;

	list_add(tablaDondeSeEncuentra->registrosPag, (registroTablaPag_t*) registroACrear);

}

//Retorna el numero de marco donde se encuentra la pagina

int colocarPaginaEnMemoria(timestamp_t timestamp, uint16_t key, char* value) { //DEBE DEVOLVER ERROR_MEMORIA_FULL si la cola esta vacia
	if(queue_is_empty(memoriaPrincipal.marcosLibres)){
		return ERROR_MEMORIA_FULL;
	}
	//wSEMAFORO
	MCB_t * marcoObjetivo = (MCB_t *) queue_pop(memoriaPrincipal.marcosLibres); //No se elimina porque el MCB tambien esta en listaAdministrativaMarcos

	void * direccionMarco = memoriaPrincipal.memoria + memoriaPrincipal.tamanioMarco * marcoObjetivo->nroMarco;

	memcpy(direccionMarco, &timestamp, sizeof(timestamp_t));

	memcpy(direccionMarco + sizeof(timestamp_t), &key, sizeof(uint16_t));

	memcpy(direccionMarco + sizeof(timestamp_t) + sizeof(uint16_t), value,
			(sizeof(char) * tamanioValue));
	//sSEMAFORO
	return marcoObjetivo->nroMarco;
}

void memoriaConUnSegmentoYUnaPagina(void) {

	//Crear un segmento
	segmento_t* segmentoEjemplo = malloc(sizeof(segmento_t));

	//Asignar path determinado
	asignarPathASegmento(segmentoEjemplo, "tablaEjemplo");

	//Crear su tabla de paginas
	segmentoEjemplo->tablaPaginas = malloc(sizeof(tabla_de_paginas_t));
	segmentoEjemplo->tablaPaginas->registrosPag = list_create();

	//Insertar pagina Ejemplo en Memoria Principal
	int indiceMarcoEjemplo = colocarPaginaEnMemoria(getCurrentTime(), 1,"Car");

	//Crear registro de pagina en la tabla

	crearRegistroEnTabla(segmentoEjemplo->tablaPaginas, indiceMarcoEjemplo);

	//Agregar segmento Ejemplo a tabla de segmentos
	list_add(tablaSegmentos.listaSegmentos, (segmento_t*) segmentoEjemplo);

}

int inicializar_memoriaPrincipal() {
	//Obtengo tamanio de memoria desde el .config
	int tamanioMemoria = atoi(fconfig.tamanio_memoria);
	//Reservo el gran bloque de memoria contigua
	memoriaPrincipal.memoria = malloc(tamanioMemoria);
	if (!memoriaPrincipal.memoria)
		return EXIT_FAILURE;

	//Fijo el tamanio de un marco en memoria
	memoriaPrincipal.tamanioMarco = sizeof(timestamp_t) + sizeof(uint16_t)
			+ (sizeof(char) * tamanioValue);

	//Cantidad de marcos que puede haber en memoria
	memoriaPrincipal.cantidadMarcos = tamanioMemoria
			/ memoriaPrincipal.tamanioMarco;

	//Inicializo cola de marcos libres y lista de administracion de marcos
	memoriaPrincipal.marcosLibres = queue_create();
	memoriaPrincipal.listaAdminMarcos = list_create();

	for (int i = 0; i < memoriaPrincipal.cantidadMarcos; ++i) {
		//Reservo espacio para un MCB
		MCB_t * marco = malloc(memoriaPrincipal.tamanioMarco);
		//Asigno a sus atributos los valores correspondientes
		marco->nroMarco = i;

		//Inserto en la cola y pila
		queue_push(memoriaPrincipal.marcosLibres, (MCB_t *) marco);
		list_add(memoriaPrincipal.listaAdminMarcos, (MCB_t *) marco);
	}

	//Creo la lista de segmentos para la tabla de segmentos
	tablaSegmentos.listaSegmentos = list_create();

	return EXIT_SUCCESS;
}

int iniciar_consola() {
	if (pthread_create(&idConsola, NULL, recibir_comandos, NULL)) {
		log_error(logger_invisible, "Memoria.c: iniciar_consola: fallo la creacion de la consola");

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int configuracion_inicial() {

	logger_visible = iniciar_logger(true);
	if (logger_visible == NULL) {
		log_error(logger_invisible, "Memoria.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true)");
		//printf(
		//		RED"Memoria.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'"STD"\n");
		return EXIT_FAILURE;
	}

	logger_invisible = iniciar_logger(false);
	if (logger_visible == NULL) {
		log_error(logger_invisible, "Memoria.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false)");
		//printf(
			//	RED"Memoria.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'"STD"\n");
		return EXIT_FAILURE;
	}

	if (inicializar_configs() == EXIT_FAILURE) {
		log_error(logger_invisible, "Memoria.c: configuracion_inicial: error en el archivo 'Memoria.config'");

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int inicializar_configs() {
	configFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);

	if (configFile == NULL) {
		printf(
				"Memoria.c: extraer_data_config: no se encontro el archivo 'Memoria.config'. Deberia estar junto al ejecutable");
		return EXIT_FAILURE;
	}

	//Config_datos_fijos
	extraer_data_fija_config();

	//Config_datos_variables
	vconfig.retardoMemoria = extraer_retardo_memoria;
	vconfig.retardoFS = extraer_retardo_FS;
	vconfig.retardoJOURNAL = extraer_retardo_JOURNAL;
	vconfig.retardoGossiping = extraer_retardo_Gossiping;
	/*
	 if(vconfig.quantum() <= 0)
	 log_error(logger_error, "Kernel.c: extraer_data_config: (Warning) el quantum con valores menores o iguales a 0 genera comportamiento indefinido");
	 */
	//TODO: Si yo hago un get de un valor que en el config no existe, va a tirar core dump. Arreglar eso.
	//La inversa no pasa nada, o sea , si agrego cosas al config y no les hago get aca no pasa nada
	//TODO: hacer que algunas se ajusten en tiempo real
	return EXIT_SUCCESS;
}

t_log* iniciar_logger(bool visible) {
	return log_create("Memoria.log", "Memoria", visible, LOG_LEVEL_INFO);
}

void extraer_data_fija_config() {
	fconfig.ip = config_get_string_value(configFile, "IP");
	fconfig.puerto = config_get_string_value(configFile, "PUERTO");
	fconfig.ip_fileSystem = config_get_string_value(configFile, "IP_FS");
	fconfig.puerto_fileSystem = config_get_string_value(configFile,
			"PUERTO_FS");
	fconfig.ip_seeds = config_get_string_value(configFile, "IP_SEEDS");
	fconfig.puerto_seeds = config_get_string_value(configFile, "PUERTO_SEEDS");
	fconfig.tamanio_memoria = config_get_string_value(configFile, "TAM_MEM");
	fconfig.numero_memoria = config_get_string_value(configFile,
			"MEMORY_NUMBER");
}

int extraer_retardo_memoria() {
	t_config *tmpConfigFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "RETARDO_MEM");
	config_destroy(tmpConfigFile);
	return res;
}

int extraer_retardo_FS() {
	t_config *tmpConfigFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "RETARDO_FS");
	config_destroy(tmpConfigFile);
	return res;
}
int extraer_retardo_JOURNAL() {
	t_config *tmpConfigFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "RETARDO_JOURNAL");
	config_destroy(tmpConfigFile);
	return res;
}
int extraer_retardo_Gossiping() {
	t_config *tmpConfigFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "RETARDO_GOSSIPING");
	config_destroy(tmpConfigFile);
	return res;
}

void mostrar_por_pantalla_config() {
	log_info(logger_visible, "IP=%s", fconfig.ip);
	log_info(logger_visible, "PUERTO=%s", fconfig.puerto);
	log_info(logger_visible, "IP_FS=%s", fconfig.ip_fileSystem);
	log_info(logger_visible, "PUERTO_FS=%s", fconfig.puerto_fileSystem);
	log_info(logger_visible, "IP_SEEDS=%s", fconfig.ip_seeds);
	log_info(logger_visible, "PUERTO_SEEDS=%s", fconfig.puerto_seeds);
	log_info(logger_visible, "RETARDO_MEM=%d", vconfig.retardoMemoria());
	log_info(logger_visible, "RETARDO_FS=%d", vconfig.retardoFS());
	log_info(logger_visible, "TAM_MEM=%s", fconfig.tamanio_memoria);
	log_info(logger_visible, "RETARDO_JOURNAL=%d", vconfig.retardoJOURNAL());
	log_info(logger_visible, "RETARDO_GOSSIPING=%d",
			vconfig.retardoGossiping());
	log_info(logger_visible, "MEMORY_NUMBER=%s", fconfig.numero_memoria);
}

void *connection_handler(void *nSocket) {
	int socket = *(int*) nSocket;
	TipoDeMensaje tipo;
	char *resultado = recv_msg(socket, &tipo);

	//Es importante realizar este chequeo devolviendo EXIT_FAILURE
	if (resultado == NULL) {
		return NULL;
	}

	printf("Hemos recibido algo!\n");

	if (tipo == COMANDO)
		comando_mostrar(parsear_comando(resultado));	//ejecutarOperacion
	if (tipo == TEXTO_PLANO)
		printf("%s\n", resultado);

	//Podríamos meter un counter y que cada X mensajes recibidos corra el gossiping
	send_msg(socket, COMANDO, resultado);

	if (resultado != NULL)
		free(resultado);

	return NULL;
}

int ejecutarOperacion(char* input) { //TODO: TIPO de retorno Resultado
	Comando *parsed = malloc(sizeof(Comando));
	*parsed = parsear_comando(input);

	//TODO: funciones pasandole userInput y parsed por si necesito enviar algo o usar algun dato parseado

	if (parsed->valido) {
		switch (parsed->keyword) {
		case SELECT:
			selectAPI(input, *parsed);
			break;
		case INSERT:
			//TODO: ojo con pasarse del tamanio maximo para value
			insertAPI(input, *parsed);
			break;
		case CREATE:
			createAPI(input, *parsed);
			break;
		case DESCRIBE:
		case DROP:
		case JOURNAL:
			printf("Entro un comando\n");
			break;
		default:
			fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n",
					parsed->keyword);
		}

		destruir_comando(*parsed);
	} else {
		fprintf(stderr, RED"La request no es valida"STD"\n");
	}
	return EXIT_SUCCESS; //MOMENTANEO
}

int iniciar_gossiping() {

	quitarCaracteresPpioFin(fconfig.ip_seeds);
	IPs = string_split(fconfig.ip_seeds, ",");

	quitarCaracteresPpioFin(fconfig.puerto_seeds);
	IPsPorts = string_split(fconfig.puerto_seeds, ",");
	for (int i = 0; IPs[i] != NULL; ++i)	//Muestro por pantalla las IP seeds
		printf("IP %d: %s\n", i, IPs[i]);

	if (pthread_create(&idGossipSend, NULL, conectar_seeds, NULL)) {
		printf(
				RED"Memoria.c: iniciar_gossiping: fallo la creacion hilo gossiping envios"STD"\n");
		return EXIT_FAILURE;
	}
	//if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
	if (pthread_create(&idGossipReciv, NULL, recibir_seeds, NULL)) {
		printf(
				RED"Memoria.c: iniciar_gossiping: fallo la creacion hilo gossiping escucha"STD"\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void *conectar_seeds(void *null) { // hilo envia a las seeds
	//pthread_detach(pthread_self());
	int puertoSocket;
	puertoSocket = conectarConSeed(IPs, IPsPorts);
	// puertoSocket = ConsultoPorMemoriasConocidas(puertoSocket);
	liberarIPs(IPs);
	liberarIPs(IPsPorts);
	for (;;) {
		// Envia mensaje a las seeds que conoce
	}
	return NULL;
}

void *recibir_seeds(void *null) { // hilo que responde con las memorias conocidas
	//pthread_detach(pthread_self());

	for (;;) {
		//Escucha todo el tiempo y cuando llegue mensaje devuelve las memorias que conoce y estan activas
	}
	return NULL;
}
void quitarCaracteresPpioFin(char* cadena) {
	for (int i = 0; i < strlen(cadena) + 1; ++i) {
		cadena[i] = cadena[i + 1];
	}
	cadena[strlen(cadena) - 1] = '\0';
}

void liberarIPs(char** IPs) {
	if (IPs != NULL) {
		for (int i = 0; IPs[i] != NULL; ++i)
			free(*(IPs + i));
		free(IPs);
	}
}

int conectarConSeed(char** IPs, char ** IPsPorts) {
	// Se conecta con la seed para hacer el gossiping
	int conteo_seeds = 0; //Static
	for (; IPs[conteo_seeds] != NULL;) {
		int socket = connect_to_server(IPs[conteo_seeds],
				IPsPorts[conteo_seeds]);
		if (socket == EXIT_FAILURE) {
			log_error(logger_invisible, "La memoria no esta activa");
			conteo_seeds++;
			return EXIT_FAILURE;
		}
		log_error(logger_invisible, "Memoria conocida. Envia rmensaje");
		conteo_seeds++;
		return ConsultoPorMemoriasConocidas(socket); //
	}
}



int ConsultoPorMemoriasConocidas(int socketSEEDS) {
	send_msg(socketSEEDS, TEXTO_PLANO, "memorias activas");

	TipoDeMensaje tipo;
	char *tamanio = recv_msg(socketSEEDS, &tipo);

	if (tipo == COMANDO)
		printf("Consulta de memorias conocidas falló. No se recibió respuesta.\n");
	if (tipo == TEXTO_PLANO)
		printf("Consulta exitosa. Se recibieron las memorias: %d\n",
				*tamanio);

	return *tamanio;
}

