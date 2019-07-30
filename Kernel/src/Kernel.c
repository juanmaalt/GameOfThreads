/*
 ============================================================================
 Name        : Kernel.c
 Author      : facundosalerno
 Version     : 1.0
 Copyright   : :)
 Description : Kernel
 ============================================================================
 */

#include "Kernel.h"

//FUNCIONES: Privadas
static int configuracion_inicial();
static int iniciar_consola();
static t_log* iniciar_logger(char *fileName, bool visibilidad, t_log_level level);
static int inicializar_configs();
static void finalizar_todos_los_hilos();
static void rutinas_de_finalizacion();
static void *inotify_service(void *null);
static void refrescar_vconfig();

int main(void) {
	//Se hacen las configuraciones iniciales para log, config y se inician semaforos
	if(configuracion_inicial() == EXIT_FAILURE)
		RETURN_ERROR("Kernel.c: main: no se pudo generar la configuracion inicial");

	log_info(logger_invisible, "=============Iniciando kernel=============");
	mostrar_por_pantalla_config(logger_visible);

	//Se inicia un proceso de consola
	if(iniciar_consola() == EXIT_FAILURE)
		RETURN_ERROR("Kernel.c: main: no se pudo levantar la consola");


	//Se entra en un estado de planificacion del cual no se sale hasta que la planificacion termine
	if(iniciar_planificador() == EXIT_FAILURE)
		RETURN_ERROR("Kernel.c: main: hubo un problema en la planificacion");

	//Rutinas de finalizacion
	rutinas_de_finalizacion();
	return EXIT_SUCCESS;
}





static int iniciar_consola(){
	if(pthread_create(&idConsola, NULL, recibir_comandos, NULL))
		RETURN_ERROR("Kernel.c: iniciar_consola: fallo la creacion de la consola")
	//No hay pthread_join. Alternativamente hay pthread_detach en la funcion recibir_comando. Hacen casi lo mismo
	return EXIT_SUCCESS;
}





static int configuracion_inicial(){
	sem_init(&disponibilidadPlanificador, 0, 0); //el ultimo valor indica el valor con el que inicia el semaforo
	sem_init(&scriptEnReady, 0, 0); //El primer valor (0) indica que afecta a hilos
	sem_init(&dormirProcesoPadre, 1, 0); //El primer valor (1) indica que afecta a procesos
	sem_init(&mutexColaReady, 0, 1);
	sem_init(&mutexMemoriasEC, 0, 1);
	sem_init(&mutexMemoriasHSC, 0, 1);
	sem_init(&mutexMemoriasSC, 0, 1);
	sem_init(&mutexMemoriasExistentes, 0, 1);
	sem_init(&mutexTablasExistentes, 0, 1);
	sem_init(&mutexMetricas, 0, 1);

	mkdir("log", 0777); //Crea la carpeta log junto al ejecutable (si ya existe no toca nada de lo que haya adentro)

	remove("log/KernelResumen.log"); //Esto define que cada ejecucion, el log se renueva
	logger_visible = iniciar_logger("log/KernelResumen.log", true, LOG_LEVEL_INFO);
	if(logger_visible == NULL)
		RETURN_ERROR("Kernel.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'");

	logger_invisible = iniciar_logger("log/KernelTodo.log", false, LOG_LEVEL_INFO);
	if(logger_visible == NULL)
		RETURN_ERROR("Kernel.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'")

	remove("log/KernelErrores.log");
	logger_error = iniciar_logger("log/KernelErrores.log", true, LOG_LEVEL_ERROR);
	if(logger_visible == NULL)
		RETURN_ERROR("Kernel.c: configuracion_inicial: error en 'logger_error = iniciar_logger(true);'");

	if(inicializar_configs() == EXIT_FAILURE)
		RETURN_ERROR("Kernel.c: configuracion_inicial: error en la extraccion de datos del archivo de configuracion");

	return EXIT_SUCCESS;
}





static t_log* iniciar_logger(char* fileName, bool visibilidad, t_log_level level) {
	return log_create(fileName, "Kernel", visibilidad, level);
}





static int inicializar_configs() {
	t_config *configFile = config_create(STANDARD_PATH_KERNEL_CONFIG);
	if(configFile == NULL)
		RETURN_ERROR("Kernel.c: extraer_data_config: no se encontro el archivo 'Kernel.config'. Deberia estar junto al ejecutable");

	//Config_datos_fijos

	if(config_get_string_value(configFile, "IP_MEMORIA") == NULL)
		RETURN_ERROR("Kernel.c: inicializar_configs: error en la extraccion del parametro IP_MEMORIA");
	fconfig.ip_memoria_principal = string_from_format(config_get_string_value(configFile, "IP_MEMORIA"));

	if(config_get_string_value(configFile, "PUERTO_MEMORIA") == NULL)
		RETURN_ERROR("Kernel.c: inicializar_configs: error en la extraccion del parametro PUERTO_MEMORIA")
	fconfig.puerto_memoria_principal = string_from_format(config_get_string_value(configFile, "PUERTO_MEMORIA"));

	if(config_get_string_value(configFile, "MULTIPROCESAMIENTO") == NULL)
		RETURN_ERROR("Kernel.c: inicializar_configs: error en la extraccion del parametro MULTIPROCESAMIENTO");
	if(!esNumerica(config_get_string_value(configFile, "MULTIPROCESAMIENTO")))
		RETURN_ERROR("Kernel.c: inicializar_configs: el parametro MULTIPROCESAMIENTO debe ser numerico")
	fconfig.multiprocesamiento = config_get_int_value(configFile, "MULTIPROCESAMIENTO");



	//Config_datos_variables
	if(config_get_string_value(configFile, "QUANTUM") == NULL)
		RETURN_ERROR("Kernel.c: inicializar_configs: error en la extraccion del parametro QUANTUM")
	if(!esNumerica(config_get_string_value(configFile, "QUANTUM")))
		RETURN_ERROR("Kernel.c: inicializar_configs: el parametro QUANTUM debe ser numerico")
	vconfig.quantum = config_get_int_value(configFile, "QUANTUM");

	if(config_get_string_value(configFile, "REFRESH_METADATA") == NULL)
		RETURN_ERROR("Kernel.c: inicializar_configs: error en la extraccion del parametro REFRESH_METADATA")
	if(!esNumerica(config_get_string_value(configFile, "REFRESH_METADATA")))
		RETURN_ERROR("Kernel.c: inicializar_configs: el parametro REFRESH_METADATA debe ser numerico")
	vconfig.refreshMetadata = config_get_int_value(configFile, "REFRESH_METADATA");

	if(config_get_string_value(configFile, "SLEEP_EJECUCION") == NULL)
		RETURN_ERROR("Kernel.c: inicializar_configs: error en la extraccion del parametro SLEEP_EJECUCION")
	if(!esNumerica(config_get_string_value(configFile, "SLEEP_EJECUCION")))
		RETURN_ERROR("Kernel.c: inicializar_configs: el parametro SLEEP_EJECUCION debe ser numerico")
	vconfig.retardo = config_get_int_value(configFile, "SLEEP_EJECUCION");

	if(pthread_create(&inotify, NULL, inotify_service, NULL))
		RETURN_ERROR("Kernel.c: inicializar_configs: no se pudo iniciar inotify");

	config_destroy(configFile);
	return EXIT_SUCCESS;
}





static void refrescar_vconfig(){
	t_config *configFile = config_create(STANDARD_PATH_KERNEL_CONFIG);
	if(configFile == NULL){
		log_error(logger_visible, "inotify_service: refrescar_vconfig: el archivo de configuracion no se encontro");
		log_error(logger_invisible, "inotify_service: refrescar_vconfig: el archivo de configuracion no se encontro");
	}

	void error(char* m){
		log_error(logger_visible, "%s", m);
		log_error(logger_invisible, "%s", m);
	}

	if(config_get_string_value(configFile, "QUANTUM") == NULL)
		error("Kernel.c: inicializar_configs: error en la extraccion del parametro QUANTUM");
	else if(!esNumerica(config_get_string_value(configFile, "QUANTUM")))
		error("Kernel.c: inicializar_configs: el parametro QUANTUM debe ser numerico");
	else vconfig.quantum = config_get_int_value(configFile, "QUANTUM");

	if(config_get_string_value(configFile, "REFRESH_METADATA") == NULL)
		error("Kernel.c: inicializar_configs: error en la extraccion del parametro REFRESH_METADATA");
	else if(!esNumerica(config_get_string_value(configFile, "REFRESH_METADATA")))
		error("Kernel.c: inicializar_configs: el parametro REFRESH_METADATA debe ser numerico");
	else vconfig.refreshMetadata = config_get_int_value(configFile, "REFRESH_METADATA");

	if(config_get_string_value(configFile, "SLEEP_EJECUCION") == NULL)
		error("Kernel.c: inicializar_configs: error en la extraccion del parametro SLEEP_EJECUCION");
	else if(!esNumerica(config_get_string_value(configFile, "SLEEP_EJECUCION")))
		error("Kernel.c: inicializar_configs: el parametro SLEEP_EJECUCION debe ser numerico");
	else vconfig.retardo = config_get_int_value(configFile, "SLEEP_EJECUCION");

	config_destroy(configFile);
}





void mostrar_por_pantalla_config(){
	log_info(logger_visible, BLU"IP_MEMORIA=%s"STD, fconfig.ip_memoria_principal);
	log_info(logger_visible, BLU"PUERTO_MEMORIA=%s"STD, fconfig.puerto_memoria_principal);
	log_info(logger_visible, BLU"QUANTUM=%d"STD, vconfig.quantum);
	log_info(logger_visible, BLU"MULTIPROCESAMIENTO=%d"STD, fconfig.multiprocesamiento);
	log_info(logger_visible, BLU"REFRESH_METADATA=%d"STD, vconfig.refreshMetadata);
	log_info(logger_visible, BLU"SLEEP_EJECUCION=%d"STD, vconfig.retardo);

	log_info(logger_invisible, "IP_MEMORIA=%s", fconfig.ip_memoria_principal);
	log_info(logger_invisible, "PUERTO_MEMORIA=%s", fconfig.puerto_memoria_principal);
	log_info(logger_invisible, "QUANTUM=%d", vconfig.quantum);
	log_info(logger_invisible, "MULTIPROCESAMIENTO=%d", fconfig.multiprocesamiento);
	log_info(logger_invisible, "REFRESH_METADATA=%d", vconfig.refreshMetadata);
	log_info(logger_invisible, "SLEEP_EJECUCION=%d", vconfig.retardo);
}





static void finalizar_todos_los_hilos(){
	pthread_cancel(idConsola);
	pthread_cancel(servicioMetricas);
	pthread_cancel(gossiping);
	void cancelar(void *hilo){
		pthread_cancel(*(pthread_t*)hilo);
	}
	list_iterate(idsExecInstances, cancelar);
}





#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
static void *inotify_service(void *null){
	void *service(){
		int fd = inotify_init();
		if(fd<0){
			printf(RED"Kernel.c: inotify_service: fallo el fd para inotify\n"STD);
			return NULL;
		}

		int watch = inotify_add_watch(fd, STANDARD_PATH_KERNEL_CONFIG, IN_MODIFY | IN_DELETE);
		if(watch<0){
			printf(RED"Kernel.c: inotify_service: fallo en el add watch para inotify\n"STD);
			return NULL;
		}
		char buf[BUF_LEN];
		int len, i = 0;
		len = read (fd, buf, BUF_LEN);
		while (i < len) {
			struct inotify_event *event;
			event = (struct inotify_event *) &buf[i];
			refrescar_vconfig();
			log_info(logger_visible, GRN"El archivo de configuracion ha cambiado"STD);
			log_info(logger_invisible, "El archivo de configuracion ha cambiado");
			mostrar_por_pantalla_config();
			//printf("wd=%d mask=%u cookie=%u len=%u\n", event->wd, event->mask, event->cookie, event->len);
			i += EVENT_SIZE + event->len;
		}
		return NULL;
	}

	for(;;)
		service();

	printf(YEL"Kernel.c: inotify_service: inotify finalizo. Ya no se podra tener un seguimiento del archivo de configuracion.\n"STD);
	return NULL;
}





static void rutinas_de_finalizacion(){
	printf(YEL"\n█▀▀▀ █▀▀█ █▀▄▀█ █▀▀ 　 █▀▀█ █▀▀ 　 ▀▀█▀▀ █░░█ █▀▀█ █▀▀ █▀▀█ █▀▀▄ █▀▀ \n█░▀█ █▄▄█ █░▀░█ █▀▀ 　 █░░█ █▀▀ 　 ░░█░░ █▀▀█ █▄▄▀ █▀▀ █▄▄█ █░░█ ▀▀█ \n▀▀▀▀ ▀░░▀ ▀░░░▀ ▀▀▀ 　 ▀▀▀▀ ▀░░ 　 ░░▀░░ ▀░░▀ ▀░▀▀ ▀▀▀ ▀░░▀ ▀▀▀░ ▀▀▀ \n\n"STD);
	//printf(GRN"Finalizando kernel..."STD"\n");
	log_info(logger_invisible, "=============Finalizando kernel=============");
	finalizar_todos_los_hilos();
	fflush(stdout);
	log_destroy(logger_visible);
	log_destroy(logger_invisible);
	log_destroy(logger_error);
}
