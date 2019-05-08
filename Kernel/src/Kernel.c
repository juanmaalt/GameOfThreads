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
	if(configuracion_inicial() == EXIT_FAILURE){
		printf("No se pudo generar la configuracion inicial\n");
		return EXIT_FAILURE;
	}
	ver_config(&config, logger_visible);

	if(iniciar_consola() == EXIT_FAILURE){
		printf("No se pudo levantar la consola\n");
		return EXIT_FAILURE;
	}

	if(iniciar_planificacion() == EXIT_FAILURE){
		printf("No se pudo comenzar a planificar\n");
		return EXIT_FAILURE;
	}

	esperar_procesos_hijos();
	finalizar_procesos_hijos();
	return 0;
}





int iniciar_planificacion(){
	pidsProcesadores = list_create();
	for(int i=0; i<config.multiprocesamiento; ++i){
		int *pid = malloc(sizeof(int)); //Lo hago asi por que los salames que hicieron la funcion list_add nada mas linkean el puntero, no le copian el valor. Por ende voy a necesitar un malloc de int por cada valor que quiera guardar, y no hacerles free de nada
		*pid = fork();
		if(*pid < 0){
			printf("Kernel.c: iniciar_planificacion: fallo la creacion de un proceso\n");
			return EXIT_FAILURE;
		}
		if (*pid == 0){
			iniciar_procesador();
		}
		list_add(pidsProcesadores, pid);
	}
	return EXIT_SUCCESS;
}





int iniciar_consola(){
	pidConsola = fork();
	if(pidConsola < 0){
		printf("Kernel.c: iniciar_consola: fallo la creacion de la consola");
		return EXIT_FAILURE;
	}
	if(pidConsola == 0){ //Esto solo lo va a poder "ver" el hijo
		recibir_comandos();
	}
	//El padre, sigue su ciclo volviendo al main
	return EXIT_SUCCESS;
}





int configuracion_inicial(){
	logger_visible = iniciar_logger(true);
	if(logger_visible == NULL){
		printf("Kernel.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'\n");
		return EXIT_FAILURE;
	}

	logger_invisible = iniciar_logger(false);
	if(logger_visible == NULL){
		printf("Kernel.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'\n");
		return EXIT_FAILURE;
	}

	t_config* configFile = leer_config();
	if(configFile == NULL){
		printf("Kernel.c: configuracion_inicial: error en el archivo 'Kernel.config'");
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
