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
	ver_config(&config, logger_visible);

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
	esperar_procesos_hijos(); //TODO: revisar la espera de procesos hijos
	finalizar_procesos_hijos(); //TODO: revisar la finalizacion de procesos hijos
	return 0;
}






int iniciar_consola(){
	pidConsola = fork();
	if(pidConsola < 0){
		printf(RED"Kernel.c: iniciar_consola: fallo la creacion de la consola"STD"\n");
		return EXIT_FAILURE;
	}
	if(pidConsola == 0){ //Esto solo lo va a poder "ver" el hijo
		recibir_comandos();
	}
	//El padre, sigue su ciclo volviendo al main
	return EXIT_SUCCESS;
}





int configuracion_inicial(){
	sem_init(&disponibilidadPlanificador, 0, 1); //Inicia mutex con valor 0
	sem_init(&scriptEnReady, 0, 0);

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

	t_config* configFile = leer_config();
	if(configFile == NULL){
		printf(RED"Kernel.c: configuracion_inicial: error en el archivo 'Kernel.config'"STD"\n");
		return EXIT_FAILURE;
	}
	extraer_data_config(&config, configFile);
	config_destroy(configFile);
	return EXIT_SUCCESS;
}





void esperar_procesos_hijos(){
	waitpid(pidConsola, NULL, 0);
	for(int i=0; i<list_size(pidsProcesadores); ++i){
		//printf("elemento %d: %d\n", i, *(int*)list_get(pidsProcesadores, i));
		waitpid(*(int*)list_get(pidsProcesadores, i), NULL, 0);
	}
}

void finalizar_procesos_hijos(){
	kill(pidConsola, SIGTERM);
	for(int i=0; i<list_size(pidsProcesadores); ++i){
		kill(*(int*)list_get(pidsProcesadores, i), SIGTERM);
	}
}

t_log* iniciar_logger(bool visible) {
	return log_create("Kernel.log", "Kernel", visible, LOG_LEVEL_INFO);
}

t_config* leer_config(){
	return config_create("Kernel.config");
}

void extraer_data_config(Config_final_data *config, t_config* configFile) {
	config->ip_memoria = calloc(strlen(config_get_string_value(configFile, "IP_MEMORIA")), sizeof(char));
	strcpy(config->ip_memoria, config_get_string_value(configFile, "IP_MEMORIA")); //Para poder liberar el configFile

	config->puerto_memoria = calloc(strlen(config_get_string_value(configFile, "PUERTO_MEMORIA")), sizeof(char));
	strcpy(config->puerto_memoria, config_get_string_value(configFile, "PUERTO_MEMORIA"));

	config->quantum = config_get_int_value(configFile, "QUANTUM");
	config->multiprocesamiento = config_get_int_value(configFile, "MULTIPROCESAMIENTO");
	config->refreshMetadata = config_get_int_value(configFile, "REFRESH_METADATA");
	config->retardo = config_get_int_value(configFile, "RETARDO");
	//TODO: Si yo hago un get de un valor que en el config no existe, va a tirar core dump. Arreglar eso.
	//La inversa no pasa nada, o sea , si agrego cosas al config y no les hago get aca no pasa nada
}


void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "IP_MEMORIA=%s", config->ip_memoria);
	log_info(logger_visible, "PUERTO_MEMORIA=%s", config->puerto_memoria);
	log_info(logger_visible, "QUANTUM=%d", config->quantum);
	log_info(logger_visible, "MULTIPROCESAMIENTO=%d", config->multiprocesamiento);
	log_info(logger_visible, "REFRESH_METADATA=%d", config->refreshMetadata);
	log_info(logger_visible, "RETARDO=%d", config->retardo);
}
