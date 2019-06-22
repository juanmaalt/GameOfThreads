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
		log_error(logger_invisible,
				"Memoria.c: main: no se pudo generar la configuracion inicial");

		return EXIT_FAILURE;
	}
	mostrar_por_pantalla_config();

	 if(realizarHandshake()==EXIT_FAILURE){
		 printf(RED"Memoria.c: main: no se pudo inicializar la memoria principal"STD"\n");
		 return EXIT_FAILURE;
	 }
/*
	tamanioValue = 4;

	pathLFS = malloc(strlen("/puntoDeMontajeQueMeDaJuanEnElHandshake/") * sizeof(char)+ 1);
	strcpy(pathLFS, "/puntoDeMontajeQueMeDaJuanEnElHandshake/");
*/
	// Inicializar la memoria principal
	if (inicializar_memoriaPrincipal() == EXIT_FAILURE) {
		log_error(logger_invisible,
				"Memoria.c: main: no se pudo inicializar la memoria principal");

		return EXIT_FAILURE;
	}
	printf("Memoria Inicializada correctamente\n");

	//TODO:GOSSIPING
	//iniciar_gossiping();

	//FUNCIONES PARA TEST DE SELECT
	memoriaConUnSegmentoYUnaPagina();

	mostrarContenidoMemoria();
	mostrarPathSegmentos();

	//Inicio consola

	if (iniciar_consola() == EXIT_FAILURE) {
		log_error(logger_invisible,
				"Memoria.c: main: no se pudo levantar la consola");

		return EXIT_FAILURE;
	}

	//Habilita el server y queda en modo en listen
	if (iniciar_serverMemoria() == EXIT_FAILURE) {
		log_error(logger_invisible,
				"Memoria.c: main: no se pudo levantar el servidor");

		return EXIT_FAILURE;
	}

	//TODO: hilo de JOURNAL


	liberarRecursos();
}

int iniciar_serverMemoria(void) {

	//TODO: separar en enable y escuchar
	int miSocket = enable_server(fconfig.ip, fconfig.puerto);
	if (miSocket == EXIT_FAILURE)
		return EXIT_FAILURE;

	log_info(logger_invisible, "Servidor encendido, esperando conexiones");

	threadConnection(miSocket, connection_handler);

	return EXIT_SUCCESS;
}

void *connection_handler(void *nSocket) {
	pthread_detach(pthread_self());
	int socket = *(int*) nSocket;
	Operacion resultado;

	resultado = recv_msg(socket);

	printf("Hemos recibido algo!\n");

	switch (resultado.TipoDeMensaje) {
	case COMANDO:
		log_info(logger_visible,"Comando recibido: %s\n",resultado.Argumentos.COMANDO.comandoParseable);
		resultado = ejecutarOperacion(resultado.Argumentos.COMANDO.comandoParseable,false);
		send_msg(socket, resultado);
		break;
	case TEXTO_PLANO:
		break;
	case REGISTRO:
		break;
	case ERROR:
		break;
	case GOSSIPING_REQUEST:
		recibir_gossiping(resultado);
		send_msg(socket, resultado);
		break;
	default:
		fprintf(stderr, RED"No se pude interpretar el enum %d"STD"\n",
				resultado.TipoDeMensaje);
	}

	//Podríamos meter un counter y que cada X mensajes recibidos corra el gossiping

	destruir_operacion(resultado);
	close(socket);
	return NULL;
}



int realizarHandshake(void) {
	lfsSocket = conectarLFS();
	log_info(logger_visible, "Conectado al LFS. Iniciando Handshake.");
	if(handshakeLFS(lfsSocket)==EXIT_FAILURE){
		return EXIT_FAILURE;
	}
	printf("TAMAÑO_VALUE= %d\n", tamanioValue);
	return EXIT_SUCCESS;
}

int handshakeLFS(int socketLFS) {
	Operacion handshake;

	handshake.TipoDeMensaje= TEXTO_PLANO;

	handshake.Argumentos.TEXTO_PLANO.texto= string_from_format("handshake");

	send_msg(socketLFS, handshake);

	destruir_operacion(handshake);

	//Recibo el tamanio
	//while((handshake = recv_msg(socketLFS)).TipoDeMensaje)
	handshake = recv_msg(socketLFS);

	switch(handshake.TipoDeMensaje){
		case TEXTO_PLANO:
			tamanioValue=atoi(handshake.Argumentos.TEXTO_PLANO.texto);
			destruir_operacion(handshake);
			break;
		case ERROR:
		case REGISTRO:
		case COMANDO:
		default:
			return EXIT_FAILURE;
	}


	//Pido el punto de montaje
	handshake.TipoDeMensaje= TEXTO_PLANO;
	handshake.Argumentos.TEXTO_PLANO.texto=string_from_format("handshake pathLFS");

	send_msg(socketLFS, handshake);

	destruir_operacion(handshake);

	//Recibo el punto de montaje
	handshake = recv_msg(socketLFS);

	switch(handshake.TipoDeMensaje){
			case TEXTO_PLANO:
				pathLFS=string_from_format(handshake.Argumentos.TEXTO_PLANO.texto);
				destruir_operacion(handshake);
				break;
			case ERROR:
			case REGISTRO:
			case COMANDO:
			default:
				return EXIT_FAILURE;
		}

	log_info(logger_visible, "El size del value es: %d", tamanioValue);
	log_info(logger_visible, "El punto de montaje es: %s",pathLFS);

	return EXIT_SUCCESS;
}

int conectarLFS() {
	//Obtiene el socket por el cual se va a conectar al LFS como cliente / * Conectarse al proceso File System
	int socket = connect_to_server(fconfig.ip_fileSystem, fconfig.puerto_fileSystem);
	if (socket == EXIT_FAILURE) {
		log_error(logger_visible,"El LFS no está levantado. Cerrar la Memoria, levantar el LFS y volver a levantar la Memoria");
		return EXIT_FAILURE;
	}
	return socket;
}

char* obtenerPath(segmento_t* segmento) {
	return segmento->pathTabla;
}

void mostrarContenidoMemoria() {
	timestamp_t timestamp;
	uint16_t key;
	char* value = malloc(sizeof(char) * tamanioValue);
	memcpy(&timestamp, memoriaPrincipal.memoria, sizeof(timestamp_t));
	memcpy(&key, memoriaPrincipal.memoria + sizeof(timestamp_t),
			sizeof(uint16_t));
	strcpy(value,
			memoriaPrincipal.memoria + sizeof(timestamp_t) + sizeof(uint16_t));

	printf("Timestamp: %llu\nKey:%d\nValue: %s\n", timestamp, key, value);

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

	registroACrear->nroPagina=list_size(tablaDondeSeEncuentra->registrosPag)-1;

	registroACrear->nroMarco = indiceMarco;

	list_add(tablaDondeSeEncuentra->registrosPag,(registroTablaPag_t*) registroACrear);

}

//Retorna el numero de marco donde se encuentra la pagina

int colocarPaginaEnMemoria(timestamp_t timestamp, uint16_t key, char* value) { //DEBE DEVOLVER ERROR_MEMORIA_FULL si la cola esta vacia
	if (queue_is_empty(memoriaPrincipal.marcosLibres)) {
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
	int indiceMarcoEjemplo = colocarPaginaEnMemoria(getCurrentTime(), 1, "Car");

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
		log_error(logger_invisible,
				"Memoria.c: iniciar_consola: fallo la creacion de la consola");

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int configuracion_inicial() {

	logger_visible = iniciar_logger(true);
	if (logger_visible == NULL) {
		log_error(logger_invisible,
				"Memoria.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true)");
		//printf(
		//		RED"Memoria.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'"STD"\n");
		return EXIT_FAILURE;
	}

	logger_invisible = iniciar_logger(false);
	if (logger_visible == NULL) {
		log_error(logger_invisible,
				"Memoria.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false)");
		//printf(
		//	RED"Memoria.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'"STD"\n");
		return EXIT_FAILURE;
	}

	if (inicializar_configs() == EXIT_FAILURE) {
		log_error(logger_invisible,
				"Memoria.c: configuracion_inicial: error en el archivo 'Memoria.config'");

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
