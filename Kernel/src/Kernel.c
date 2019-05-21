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

int main(void) {
	//Se hacen las configuraciones iniciales para log, config y se inician semaforos
	if(configuracion_inicial() == EXIT_FAILURE)
		RETURN_ERROR("Kernel.c: main: no se pudo generar la configuracion inicial");

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
	sem_init(&scriptEnReady, 0, 0);
	sem_init(&dormirProcesoPadre, 1, 0);
	sem_init(&extraerDeReadyDeAUno, 0, 1);
	sem_init(&meterEnReadyDeAUno, 0, 1);

	logger_visible = iniciar_logger("KernelResumen.log", true, LOG_LEVEL_INFO);
	if(logger_visible == NULL)
		RETURN_ERROR("Kernel.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'");

	logger_invisible = iniciar_logger("KernelTodo.log", false, LOG_LEVEL_INFO);
	if(logger_visible == NULL)
		RETURN_ERROR("Kernel.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'")

	logger_error = iniciar_logger("KernelErrores.log", true, LOG_LEVEL_ERROR);
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
	configFile = config_create(STANDARD_PATH_KERNEL_CONFIG);
	if(configFile == NULL)
		RETURN_ERROR("Kernel.c: extraer_data_config: no se encontro el archivo 'Kernel.config'. Deberia estar junto al ejecutable")

	//Config_datos_fijos
	fconfig.ip_memoria = config_get_string_value(configFile, "IP_MEMORIA");
	fconfig.puerto_memoria = config_get_string_value(configFile, "PUERTO_MEMORIA");
	fconfig.multiprocesamiento = config_get_int_value(configFile, "MULTIPROCESAMIENTO");

	//Config_datos_variables
	vconfig.quantum = extraer_quantum_config;
	vconfig.refreshMetadata = extraer_refresMetadata_config;
	vconfig.retardo = extraer_retardo_config;

	if(vconfig.quantum() <= 0)
		log_error(logger_error, "Kernel.c: extraer_data_config: (Warning) el quantum con valores menores o iguales a 0 genera comportamiento indefinido");
	//TODO: Si yo hago un get de un valor que en el config no existe, va a tirar core dump. Arreglar eso.
	//La inversa no pasa nada, o sea , si agrego cosas al config y no les hago get aca no pasa nada

	//TODO: hacer que algunas se ajusten en tiempo real
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
	int res = config_get_int_value(configFile, "REFRESH_METADATA");
	config_destroy(tmpConfigFile);
	return res;
}
static int extraer_retardo_config(){
	t_config *tmpConfigFile = config_create(STANDARD_PATH_KERNEL_CONFIG);
	int res = config_get_int_value(configFile, "RETARDO");
	config_destroy(tmpConfigFile);
	return res;
}





void mostrar_por_pantalla_config(){
	log_info(logger_visible, "IP_MEMORIA=%s", fconfig.ip_memoria);
	log_info(logger_visible, "PUERTO_MEMORIA=%s", fconfig.puerto_memoria);
	log_info(logger_visible, "QUANTUM=%d", vconfig.quantum());
	log_info(logger_visible, "MULTIPROCESAMIENTO=%d", fconfig.multiprocesamiento);
	log_info(logger_visible, "REFRESH_METADATA=%d", vconfig.refreshMetadata());
	log_info(logger_visible, "RETARDO=%d", vconfig.retardo());
}





static void finalizar_todos_los_hilos(){
	pthread_cancel(idConsola);
	void cancelar(void *hilo){
		pthread_cancel(*(pthread_t*)hilo);
	}
	list_iterate(idsExecInstances, cancelar);
}





static void rutinas_de_finalizacion(){
	finalizar_todos_los_hilos();
	fflush(stdout);
	config_destroy(configFile);
	log_destroy(logger_visible);
	log_destroy(logger_invisible);
	log_destroy(logger_error);
}
