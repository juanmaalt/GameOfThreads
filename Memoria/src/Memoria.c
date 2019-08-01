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

static void refrescar_vconfig();
static void *inotify_service(void *null);
static void finalizar_todos_los_hilos();

int main(void) {
	//Se hacen las configuraciones iniciales para log y config
	if (configuracion_inicial() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo generar la configuracion inicial");

		return EXIT_FAILURE;
	}
	log_info(logger_invisible, "=============Iniciando Memoria=============");
	mostrar_por_pantalla_config();

	if (realizarHandshake() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo inicializar la memoria principal");
		return EXIT_FAILURE;
	}

	/*tamanioValue = 4;

	 pathLFS = malloc(strlen("/puntoDeMontajeQueMeDaJuanEnElHandshake/") * sizeof(char)+ 1);
	 strcpy(pathLFS, "/puntoDeMontajeQueMeDaJuanEnElHandshake/");
	 */
	// Inicializar la memoria principal
	if (inicializar_memoriaPrincipal() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo inicializar la memoria principal");

		return EXIT_FAILURE;
	}

	log_info(logger_visible, "Memoria Inicializada correctamente");

	//TODO:GOSSIPING
	iniciar_gossiping();

	//Inicio consola
	if (iniciar_consola() == EXIT_FAILURE) {
		log_error(logger_error,
				"Memoria.c: main: no se pudo levantar la consola");

		return EXIT_FAILURE;
	}

	if (iniciar_Journal() == EXIT_FAILURE) {
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
	log_info(logger_invisible, "=============Finalizando Memoria=============");
	finalizar_todos_los_hilos();
	liberarRecursos();

}

void *realizarJournal(void* null) {
	pthread_detach(pthread_self());
	Operacion retorno;
	while (1) {
		usleep(vconfig.retardoJOURNAL * 1000);

		log_info(logger_invisible,
				"Memoria.c: realizarJournal: Inicio Journal automatico");
		//TODO: WAIT MUTEX
		pthread_mutex_lock(&mutexJournalSimultaneo);
		retorno=journalAPI();
		pthread_mutex_unlock(&mutexJournalSimultaneo);
		destruir_operacion(retorno);
	}
}

int iniciar_Journal(void) {
	if (pthread_create(&idJournal, NULL, realizarJournal, NULL)) {
		log_error(logger_error,
				"Memoria.c: iniciar_consola: fallo el hilo de JOURNAL automatico");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int iniciar_serverMemoria(void) {

	//TODO: separar en enable y escuchar
	memorySocket = enable_server(fconfig.ip, fconfig.puerto);
	if (memorySocket == EXIT_FAILURE)
		return EXIT_FAILURE;

	log_info(logger_invisible,
			"Memoria.c: iniciar_serverMemoria: Servidor encendido, esperando conexiones");

	threadConnection(memorySocket, connection_handler);

	return EXIT_SUCCESS;
}

void *connection_handler(void *nSocket) {
	pthread_detach(pthread_self());
	int socket = *(int*) nSocket;
	Operacion resultado;
	char *copiaComandoParseable = NULL;
	resultado = recv_msg(socket);

	switch (resultado.TipoDeMensaje) {
	case COMANDO:
		log_info(logger_visible, "Request recibido por SOCKET: %s",
				resultado.Argumentos.COMANDO.comandoParseable);
		log_info(logger_invisible, "Request recibido por SOCKET: %s",
						resultado.Argumentos.COMANDO.comandoParseable);
		copiaComandoParseable = string_from_format(resultado.Argumentos.COMANDO.comandoParseable);

		resultado = ejecutarOperacion(resultado.Argumentos.COMANDO.comandoParseable, false);

		loggearRetorno(resultado, logger_invisible);
		send_msg(socket, resultado);
		if(resultado.TipoDeMensaje== ERROR_MEMORIAFULL){
			retenerRequestPorMemoriaFull();
			resultado = ejecutarOperacion(copiaComandoParseable, false);
			send_msg(socket, resultado);
		}
		free(copiaComandoParseable);

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
	free(nSocket);
	return NULL;
}

int realizarHandshake(void) {
	int lfsSocket = conectarLFS();
	log_info(logger_visible, "Conectado al LFS. Iniciando Handshake.");
	if (handshakeLFS(lfsSocket) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	//printf("TAMAÑO_VALUE= %d\n", tamanioValue);
	log_info(logger_visible, "Handshake realizado correctamente");

	close(lfsSocket);
	return EXIT_SUCCESS;
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
		pthread_mutex_lock(&mutexColaMarcos);

		queue_push(memoriaPrincipal.marcosLibres, (MCB_t *) marco);

		pthread_mutex_unlock(&mutexColaMarcos);

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
	//INICIALIZAR SEMAFOROS
	journal.enEspera = 0;
	journal.ejecutando = 0;
	journal.retenidosMemFull = 0;
	sem_init(&journal.sem, 0, 1);
	sem_init(&journal.semRequest, 0, 1);
	sem_init(&journal.memoriaFull, 0, 0);

	pthread_mutex_init(&mutexJournalSimultaneo,NULL);
	pthread_mutex_init(&mutexFull, NULL);
	pthread_mutex_init(&mutexMemoria, NULL);
	pthread_mutex_init(&mutexTablaSegmentos, NULL);
	pthread_mutex_init(&mutexColaMarcos,NULL);
	pthread_mutex_init(&mutexEnEspera, NULL);
	pthread_mutex_init(&mutexEjecutando, NULL);

	mkdir("Logs", 0777); //Crea la carpeta Logs junto al ejecutable (si ya existe no toca nada de lo que haya adentro)

	remove("Logs/MemoriaResumen.log"); //Esto define que cada ejecucion, el log se renueva

	logger_visible = iniciar_logger("Logs/MemoriaResumen.log", true,
			LOG_LEVEL_INFO);
	if (logger_visible == NULL)
		RETURN_ERROR(
				"Memoria.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'");

	logger_invisible = iniciar_logger("Logs/MemoriaTodo.log", false,
			LOG_LEVEL_INFO);
	if (logger_invisible == NULL)
		RETURN_ERROR(
				"Memoria.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'");

	remove("Logs/MemoriaErrores.log");
	logger_error = iniciar_logger("Logs/MemoriaErrores.log", true,
			LOG_LEVEL_ERROR);
	if (logger_error == NULL)
		RETURN_ERROR(
				"Memoria.c: configuracion_inicial: error en 'logger_error = iniciar_logger(true);'");

	remove("Logs/MemoriaGossiping.log");
	logger_gossiping = iniciar_logger("Logs/MemoriaGossiping.log", false,
			LOG_LEVEL_INFO);
	if (logger_gossiping == NULL)
		RETURN_ERROR(
				"Memoria.c: configuracion_inicial: error en 'logger_gossiping = iniciar_logger(false);'");
	if (inicializar_configs() == EXIT_FAILURE)
		RETURN_ERROR(
				"Memoria.c: configuracion_inicial: error en la extraccion de datos del archivo de configuracion");

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
	//Retardo Memoria
	if (config_get_string_value(configFile, "RETARDO_MEM") == NULL)
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: error en la extraccion del parametro RETARDO_MEM")
	if (!esNumerica(config_get_string_value(configFile, "RETARDO_MEM"), false))
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: el parametro RETARDO_MEM debe ser numerico")
	vconfig.retardoMemoria = config_get_int_value(configFile, "RETARDO_MEM");

	//Retardo FS
	if (config_get_string_value(configFile, "RETARDO_FS") == NULL)
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: error en la extraccion del parametro RETARDO_FS")
	if (!esNumerica(config_get_string_value(configFile, "RETARDO_FS"), false))
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: el parametro RETARDO_FS debe ser numerico")
	vconfig.retardoFS = config_get_int_value(configFile, "RETARDO_FS");

	//Retardo JOURNAL
	if (config_get_string_value(configFile, "RETARDO_JOURNAL") == NULL)
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: error en la extraccion del parametro RETARDO_JOURNAL")
	if (!esNumerica(config_get_string_value(configFile, "RETARDO_JOURNAL"), false))
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: el parametro RETARDO_JOURNAL debe ser numerico")
	vconfig.retardoJOURNAL = config_get_int_value(configFile,
			"RETARDO_JOURNAL");

	//Retardo GOSSIPING
	if (config_get_string_value(configFile, "RETARDO_GOSSIPING") == NULL)
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: error en la extraccion del parametro RETARDO_GOSSIPING")
	if (!esNumerica(config_get_string_value(configFile, "RETARDO_GOSSIPING"), false))
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: el parametro RETARDO_GOSSIPING debe ser numerico")
	vconfig.retardoGossiping = config_get_int_value(configFile,
			"RETARDO_GOSSIPING");


	if (pthread_create(&inotify, NULL, inotify_service, NULL))
		RETURN_ERROR(
				"Memoria.c: inicializar_configs: no se pudo iniciar inotify");

	return EXIT_SUCCESS;
}

t_log* iniciar_logger(char* fileName, bool visibilidad, t_log_level level) {
	return log_create(fileName, "Memoria", visibilidad, level);
}

void loggearRetorno(Operacion retorno, t_log * logger) {
	switch (retorno.TipoDeMensaje) {
	case REGISTRO:
		log_info(logger, "Timestamp: %llu  Key:%d  Value: %s\n",
				retorno.Argumentos.REGISTRO.timestamp,
				retorno.Argumentos.REGISTRO.key,
				retorno.Argumentos.REGISTRO.value);
		return;
	case TEXTO_PLANO:
		log_info(logger, "Resultado: %s\n",
				retorno.Argumentos.TEXTO_PLANO.texto);
		return;
	case ERROR:
		log_error(logger_error, "%s \n", retorno.Argumentos.ERROR.mensajeError);
		return;
	case ERROR_JOURNAL:
		log_error(logger_error, "La request no se proceso. Realizando Journal");
		return;
	case ERROR_MEMORIAFULL:
		log_error(logger_error, "MEMORIA FULL, REALIZAR JOURNAL");
		return;
	case COMANDO:
		return;
	case GOSSIPING_REQUEST:
		log_info(logger, "GOSSIPING_REQUEST");
		return;
	case GOSSIPING_REQUEST_KERNEL:
		log_info(logger, "GOSSIPING_REQUEST_KERNEL");
		return;
	case DESCRIBE_REQUEST:
		log_info(logger, "DESCRIBE: %s", retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
		return;
	default:
		log_error(logger, "Memoria.c: No cumple con el tipo de mensaje esperado");
		return;
	}
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

void mostrar_por_pantalla_config() {
	log_info(logger_visible, BLU"IP=%s", fconfig.ip);
	log_info(logger_visible, BLU"PUERTO=%s", fconfig.puerto);
	log_info(logger_visible, BLU"IP_FS=%s", fconfig.ip_fileSystem);
	log_info(logger_visible, BLU"PUERTO_FS=%s", fconfig.puerto_fileSystem);
	log_info(logger_visible, BLU"IP_SEEDS=%s", fconfig.ip_seeds);
	log_info(logger_visible, BLU"PUERTO_SEEDS=%s", fconfig.puerto_seeds);
	log_info(logger_visible, BLU"RETARDO_MEM=%d", vconfig.retardoMemoria);
	log_info(logger_visible, BLU"RETARDO_FS=%d", vconfig.retardoFS);
	log_info(logger_visible, BLU"TAM_MEM=%s", fconfig.tamanio_memoria);
	log_info(logger_visible, BLU"RETARDO_JOURNAL=%d", vconfig.retardoJOURNAL);
	log_info(logger_visible, BLU"RETARDO_GOSSIPING=%d",
			vconfig.retardoGossiping);
	log_info(logger_visible, BLU"MEMORY_NUMBER=%s", fconfig.numero_memoria);

	log_info(logger_invisible, "IP=%s", fconfig.ip);
	log_info(logger_invisible, "PUERTO=%s", fconfig.puerto);
	log_info(logger_invisible, "IP_FS=%s", fconfig.ip_fileSystem);
	log_info(logger_invisible, "PUERTO_FS=%s", fconfig.puerto_fileSystem);
	log_info(logger_invisible, "IP_SEEDS=%s", fconfig.ip_seeds);
	log_info(logger_invisible, "PUERTO_SEEDS=%s", fconfig.puerto_seeds);
	log_info(logger_invisible, "RETARDO_MEM=%d", vconfig.retardoMemoria);
	log_info(logger_invisible, "RETARDO_FS=%d", vconfig.retardoFS);
	log_info(logger_invisible, "TAM_MEM=%s", fconfig.tamanio_memoria);
	log_info(logger_invisible, "RETARDO_JOURNAL=%d", vconfig.retardoJOURNAL);
	log_info(logger_invisible, "RETARDO_GOSSIPING=%d",
			vconfig.retardoGossiping);
	log_info(logger_invisible, "MEMORY_NUMBER=%s", fconfig.numero_memoria);
}

static void refrescar_vconfig() {
	t_config *tmpConfigFile = config_create(STANDARD_PATH_MEMORIA_CONFIG);
	if (tmpConfigFile == NULL) {
		log_error(logger_visible,
				"inotify_service: refrescar_vconfig: el archivo de configuracion no se encontro");
		log_error(logger_invisible,
				"inotify_service: refrescar_vconfig: el archivo de configuracion no se encontro");
	}

	vconfig.retardoMemoria = config_get_int_value(tmpConfigFile, "RETARDO_MEM");
	vconfig.retardoFS = config_get_int_value(tmpConfigFile, "RETARDO_FS");
	vconfig.retardoJOURNAL = config_get_int_value(tmpConfigFile,
			"RETARDO_JOURNAL");
	vconfig.retardoGossiping = config_get_int_value(tmpConfigFile,
			"RETARDO_GOSSIPING");
	config_destroy(tmpConfigFile);
}

#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
static void *inotify_service(void *null) {
	void *service() {
		int fd = inotify_init();
		if (fd < 0) {
			printf(
			RED"Memoria.c: inotify_service: fallo el fd para inotify\n"STD);
			return NULL;
		}

		int watch = inotify_add_watch(fd, STANDARD_PATH_MEMORIA_CONFIG,
		IN_MODIFY | IN_DELETE);
		if (watch < 0) {
			printf(
					RED"Memoria.c: inotify_service: fallo en el add watch para inotify\n"STD);
			return NULL;
		}
		char buf[BUF_LEN];
		int len, i = 0;
		len = read(fd, buf, BUF_LEN);
		while (i < len) {
			struct inotify_event *event;
			event = (struct inotify_event *) &buf[i];
			refrescar_vconfig();
			log_info(logger_visible,
			GRN"El archivo de configuracion ha cambiado"STD);
			log_info(logger_invisible,
					"El archivo de configuracion ha cambiado");
			mostrar_por_pantalla_config();
			//printf("wd=%d mask=%u cookie=%u len=%u\n", event->wd, event->mask, event->cookie, event->len);
			i += EVENT_SIZE + event->len;
		}
		return NULL;
	}

	for (;;)
		service();

	printf(
			YEL"Memoria.c: inotify_service: inotify finalizo. Ya no se podra tener un seguimiento del archivo de configuracion.\n"STD);
	return NULL;
}

static void finalizar_todos_los_hilos(){
	pthread_cancel(idConsola);
	pthread_cancel(idJournal);
	pthread_cancel(inotify);

}
