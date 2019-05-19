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
	printf(RED"manc.c: (Warning) se esta finalizando el Kernel"STD"\n");
	return 0;
}






int iniciar_consola(){
	if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
		printf(RED"Kernel.c: iniciar_consola: fallo la creacion de la consola"STD"\n");
		return EXIT_FAILURE;
	}
	//No hay pthread_join. Alternativamente hay pthread_detach en la funcion recibir_comando. Hacen casi lo mismo
	return EXIT_SUCCESS;
}





int configuracion_inicial(){
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

	t_config* configFile = leer_config();
	if(configFile == NULL){
		printf(RED"Kernel.c: configuracion_inicial: error en el archivo 'Kernel.config'"STD"\n");
		return EXIT_FAILURE;
	}
	extraer_data_config(&config, configFile);
	config_destroy(configFile);
	return EXIT_SUCCESS;
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
	if(config->quantum <= 0)
		printf(RED"main.c: extraer_data_config: (Warning) el quantum con valores menores o iguales a 0 genera comportamiento indefinido"STD"\n");
	//TODO: Si yo hago un get de un valor que en el config no existe, va a tirar core dump. Arreglar eso.
	//La inversa no pasa nada, o sea , si agrego cosas al config y no les hago get aca no pasa nada

	//TODO: hacer que algunas se ajusten en tiempo real
}


void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "IP_MEMORIA=%s", config->ip_memoria);
	log_info(logger_visible, "PUERTO_MEMORIA=%s", config->puerto_memoria);
	log_info(logger_visible, "QUANTUM=%d", config->quantum);
	log_info(logger_visible, "MULTIPROCESAMIENTO=%d", config->multiprocesamiento);
	log_info(logger_visible, "REFRESH_METADATA=%d", config->refreshMetadata);
	log_info(logger_visible, "RETARDO=%d", config->retardo);
}
