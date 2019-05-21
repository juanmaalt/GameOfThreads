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
	if(configuracion_inicial() == EXIT_FAILURE){
		printf(RED"Kernel.c: main: no se pudo generar la configuracion inicial"STD"\n");
		return EXIT_FAILURE;
	}
	mostrar_por_pantalla_config(logger_visible);

	//Se inicia un proceso de consola
	if(iniciar_consola() == EXIT_FAILURE){
		printf(RED"Kernel.c: main: no se pudo levantar la consola"STD"\n");
		return EXIT_FAILURE;
	}

	//Se entra en un estado de planificacion del cual no se sale hasta que la planificacion termine
	if(iniciar_planificador() == EXIT_FAILURE){
		printf(RED"Kernel.c: main: hubo un problema en la planificacion"STD"\n");
		return EXIT_FAILURE;
	}

	//Rutinas de finalizacion
	rutinas_de_finalizacion();
	return EXIT_SUCCESS;
}





static int iniciar_consola(){
	if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
		printf(RED"Kernel.c: iniciar_consola: fallo la creacion de la consola"STD"\n");
		return EXIT_FAILURE;
	}
	//No hay pthread_join. Alternativamente hay pthread_detach en la funcion recibir_comando. Hacen casi lo mismo
	return EXIT_SUCCESS;
}





static int configuracion_inicial(){
	sem_init(&disponibilidadPlanificador, 0, 0); //el ultimo valor indica el valor con el que inicia el semaforo
	sem_init(&scriptEnReady, 0, 0);
	sem_init(&dormirProcesoPadre, 1, 0);
	sem_init(&extraerDeReadyDeAUno, 0, 1);
	sem_init(&meterEnReadyDeAUno, 0, 1);

	logger_visible = iniciar_logger(true);
	if(logger_visible == NULL){
		printf(RED"Kernel.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'"STD"\n");
		return EXIT_FAILURE;
	}

	logger_invisible = iniciar_logger(false);
	if(logger_visible == NULL){
		printf(RED"Kernel.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'"STD"\n");
		return EXIT_FAILURE;
	}

	logger_error = iniciar_logger(true);
	if(logger_visible == NULL){
		printf(RED"Kernel.c: configuracion_inicial: error en 'logger_error = iniciar_logger(true);'"STD"\n");
		return EXIT_FAILURE;
	}

	if(inicializar_configs() == EXIT_FAILURE){
		printf(RED"Kernel.c: configuracion_inicial: error en la extraccion de datos del archivo de configuracion"STD"\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}





static t_log* iniciar_logger(bool visible) {
	return log_create("Kernel.log", "Kernel", visible, LOG_LEVEL_INFO);
}





static int inicializar_configs() {
	configFile = config_create("Kernel.config");
	if(configFile == NULL){
		printf(RED"Kernel.c: extraer_data_config: no se encontro el archivo 'Kernel.config'. Deberia estar junto al ejecutable"STD"\n");
		return EXIT_FAILURE;
	}

	//Config_datos_fijos
	fconfig.ip_memoria = config_get_string_value(configFile, "IP_MEMORIA");
	fconfig.puerto_memoria = config_get_string_value(configFile, "PUERTO_MEMORIA");
	fconfig.multiprocesamiento = config_get_int_value(configFile, "MULTIPROCESAMIENTO");

	//Config_datos_variables
	vconfig.quantum = extraer_quantum_config;
	vconfig.refreshMetadata = extraer_refresMetadata_config;
	vconfig.retardo = extraer_retardo_config;

	if(vconfig.quantum() <= 0)
		printf(RED"Kernel.c: extraer_data_config: (Warning) el quantum con valores menores o iguales a 0 genera comportamiento indefinido"STD"\n");
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





void mostrar_por_pantalla_config(t_log* logger_visible){
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
	printf(RED"(Warning) se esta finalizando el Kernel"STD"\n");
	finalizar_todos_los_hilos();
	fflush(stdout);
	config_destroy(configFile);
	log_destroy(logger_visible);
	log_destroy(logger_invisible);
	log_destroy(logger_error);
}
