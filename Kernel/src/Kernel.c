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
static int extraer_quantum_config();
static int extraer_refresMetadata_config();
static int extraer_retardo_config();
static void finalizar_todos_los_hilos();
static void rutinas_de_finalizacion();

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
	sem_init(&extraerDeReadyDeAUno, 0, 1);
	sem_init(&meterEnReadyDeAUno, 0, 1);

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
	fconfig.ip_memoria_principal = malloc(sizeof(char) * strlen(config_get_string_value(configFile, "IP_MEMORIA"))+1);
	strcpy(fconfig.ip_memoria_principal, config_get_string_value(configFile, "IP_MEMORIA"));

	fconfig.puerto_memoria_principal = malloc(sizeof(char) * strlen(config_get_string_value(configFile, "PUERTO_MEMORIA"))+1);
	strcpy(fconfig.puerto_memoria_principal, config_get_string_value(configFile, "PUERTO_MEMORIA"));

	fconfig.multiprocesamiento = config_get_int_value(configFile, "MULTIPROCESAMIENTO");

	config_destroy(configFile);

	//Config_datos_variables
	vconfig.quantum = extraer_quantum_config;
	vconfig.refreshMetadata = extraer_refresMetadata_config;
	vconfig.retardo = extraer_retardo_config;

	if(fconfig.multiprocesamiento <= 0){
		log_error(logger_error, "Kernel.c: extraer_data_config: (Warning) el multiprocesamiento con valores menores o iguales a 0 genera comportamiento indefinido");
		log_error(logger_error, "Kernel.c: extraer_data_config: (Warning) el multiprocesamiento con valores menores o iguales a 0 genera comportamiento indefinido");
	}
	if(vconfig.quantum() <= 0){
		log_error(logger_error, "Kernel.c: extraer_data_config: (Warning) el quantum con valores menores o iguales a 0 genera comportamiento indefinido");
	log_error(logger_error, "Kernel.c: extraer_data_config: (Warning) el quantum con valores menores o iguales a 0 genera comportamiento indefinido");
	}
	return EXIT_SUCCESS;
}





static int extraer_quantum_config(){
	t_config *tmpConfigFile = config_create(STANDARD_PATH_KERNEL_CONFIG); //Muhcho overhead pero no encontre otra forma, ya que el config una vez que esta abierto, no recibe las modificaciones en tiempo real. Hay que abrirlo y cerrarlo cada vez
	int res = config_get_int_value(tmpConfigFile, "QUANTUM");
	config_destroy(tmpConfigFile);
	return res;
}
static int extraer_refresMetadata_config(){
	t_config *tmpConfigFile = config_create(STANDARD_PATH_KERNEL_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "REFRESH_METADATA");
	config_destroy(tmpConfigFile);
	return res;
}
static int extraer_retardo_config(){
	t_config *tmpConfigFile = config_create(STANDARD_PATH_KERNEL_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "RETARDO");
	config_destroy(tmpConfigFile);
	return res;
}





void mostrar_por_pantalla_config(){
	log_info(logger_visible, BLU"IP_MEMORIA=%s"STD, fconfig.ip_memoria_principal);
	log_info(logger_visible, BLU"PUERTO_MEMORIA=%s"STD, fconfig.puerto_memoria_principal);
	log_info(logger_visible, BLU"QUANTUM=%d"STD, vconfig.quantum());
	log_info(logger_visible, BLU"MULTIPROCESAMIENTO=%d"STD, fconfig.multiprocesamiento);
	log_info(logger_visible, BLU"REFRESH_METADATA=%d"STD, vconfig.refreshMetadata());
	log_info(logger_visible, BLU"RETARDO=%d"STD, vconfig.retardo());

	log_info(logger_invisible, "IP_MEMORIA=%s", fconfig.ip_memoria_principal);
	log_info(logger_invisible, "PUERTO_MEMORIA=%s", fconfig.puerto_memoria_principal);
	log_info(logger_invisible, "QUANTUM=%d", vconfig.quantum());
	log_info(logger_invisible, "MULTIPROCESAMIENTO=%d", fconfig.multiprocesamiento);
	log_info(logger_invisible, "REFRESH_METADATA=%d", vconfig.refreshMetadata());
	log_info(logger_invisible, "RETARDO=%d", vconfig.retardo());
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
