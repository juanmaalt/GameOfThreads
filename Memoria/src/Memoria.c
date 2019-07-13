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
		log_error(logger_error,
				"Memoria.c: main: no se pudo generar la configuracion inicial");

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

	pathLFS = malloc(strlen("/puntoDeMontajeQueMeDaJuanEnElHandshake/") * sizeof(char)+ 1);
	strcpy(pathLFS, "/puntoDeMontajeQueMeDaJuanEnElHandshake/");

	// Inicializar la memoria principal
	if (inicializar_memoriaPrincipal() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo inicializar la memoria principal");

		return EXIT_FAILURE;
	}

	log_info(logger_visible,"Memoria Inicializada correctamente\n");

	//TODO:GOSSIPING
	iniciar_gossiping();

	//FUNCIONES PARA TEST DE SELECT TODO: ELIMINAR
	memoriaConUnSegmentoYUnaPagina();

	mostrarContenidoMemoria();
	mostrarPathSegmentos();

	//Inicio consola

	if (iniciar_consola() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo levantar la consola");

		return EXIT_FAILURE;
	}

	//TODO: hilo de JOURNAL
	if(iniciar_Journal() == EXIT_FAILURE){
		log_error(logger_error,
						"Memoria.c: main: no se pudo iniciar el hilo journal");

				return EXIT_FAILURE;
	}

	//Habilita el server y queda en modo en listen
	if (iniciar_serverMemoria() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo levantar el servidor");

		return EXIT_FAILURE;
	}



	liberarRecursos();
}

void *realizarJournal(void* null){
	pthread_detach(pthread_self());
	while(1){
		usleep(vconfig.retardoJOURNAL() * 1000);

		log_info(logger_invisible,"Memoria.c: realizarJournal: Inicio Journal automatico");
		journalAPI();
	}
}

int iniciar_Journal(void){
	if (pthread_create(&idJournal, NULL, realizarJournal, NULL)) {
			log_error(logger_error,
					"Memoria.c: iniciar_consola: fallo el hilo de JOURNAL automatico");
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}




int iniciar_serverMemoria(void) {

	//TODO: separar en enable y escuchar
	int miSocket = enable_server(fconfig.ip, fconfig.puerto);
	if (miSocket == EXIT_FAILURE)
		return EXIT_FAILURE;

	log_info(logger_invisible, "Memoria.c: iniciar_serverMemoria: Servidor encendido, esperando conexiones");

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
		log_info(logger_visible,"Request recibido por SOCKET: %s\n",resultado.Argumentos.COMANDO.comandoParseable);
		resultado = ejecutarOperacion(resultado.Argumentos.COMANDO.comandoParseable,false);
		//log_info(logger_invisible,"Memoria.c: connection_handler: Resultado select Timestamp %llu Key %s value %s \n",resultado.Argumentos.REGISTRO.timestamp,resultado.Argumentos.REGISTRO.key,resultado.Argumentos.REGISTRO.value);
		send_msg(socket, resultado);
		break;
	case TEXTO_PLANO:
		break;
	case REGISTRO:
		break;
	case ERROR:
		break;
	case GOSSIPING_REQUEST:
		resultado = recibir_gossiping(resultado);
		send_msg(socket, resultado);

		break;
	case GOSSIPING_REQUEST_KERNEL:
		resultado = recibir_gossiping(resultado);
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
	log_info(logger_visible, "Handshake realizado correctamente");
	return EXIT_SUCCESS;
}


//TODO: FUNCION A ELIMINAR >>> USADA PARA TEST

void memoriaConUnSegmentoYUnaPagina(void) { //TODO: se podrian usar las de manejo de memoria...

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

	crearRegistroEnTabla(segmentoEjemplo->tablaPaginas, indiceMarcoEjemplo, false);

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
		log_error(logger_error,
				"Memoria.c: iniciar_consola: fallo la creacion de la consola");

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int configuracion_inicial() {
	//INICIALIZAR SEMAFOROS ACA

	mkdir("Logs", 0777); //Crea la carpeta Logs junto al ejecutable (si ya existe no toca nada de lo que haya adentro)

	remove("Logs/MemoriaResumen.log"); //Esto define que cada ejecucion, el log se renueva

	logger_visible = iniciar_logger("Logs/MemoriaResumen.log", true, LOG_LEVEL_INFO);
	if(logger_visible == NULL)
		RETURN_ERROR("Memoria.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'");

	logger_invisible = iniciar_logger("Logs/MemoriaTodo.log", false, LOG_LEVEL_INFO);
	if(logger_invisible == NULL)
		RETURN_ERROR("Memoria.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'");

	remove("Logs/MemoriaErrores.log");
	logger_error = iniciar_logger("Logs/MemoriaErrores.log", true, LOG_LEVEL_ERROR);
	if(logger_error == NULL)
		RETURN_ERROR("Memoria.c: configuracion_inicial: error en 'logger_error = iniciar_logger(true);'");

	if (inicializar_configs() == EXIT_FAILURE)
		RETURN_ERROR("Memoria.c: configuracion_inicial: error en la extraccion de datos del archivo de configuracion");

	return EXIT_SUCCESS;
}

int inicializar_configs() {
	configFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);

	if (configFile == NULL) {
		log_error(logger_error,
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

t_log* iniciar_logger(char* fileName, bool visibilidad, t_log_level level) {
	return log_create(fileName, "Memoria", visibilidad, level);
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
