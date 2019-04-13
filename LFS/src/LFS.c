/*
 ============================================================================
 Name        : LFS.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Lissandra File System
 ============================================================================
 */

#include "LFS.h"

int main(void) {
	Config_final_data config;

	//Funciones .log
	t_log* logger_visible = iniciar_logger();

	//Funciones .config
	t_config* configFile = leer_config();
	levantar_config(&config, configFile);
	ver_config(&config, logger_visible);
	//Meter funcion para levantar las variables de tiempo retardo y tiempo_dump


	int kernelSocket = connect_to_server("127.0.0.1", "8001");

	if(kernelSocket == EXIT_FAILURE){
		fprintf(stderr, "No se pudo conectar al server\n");
		return EXIT_FAILURE;
	}

	char* input = readline("Mandele un comando al kernel> ");
	Comando parsed = parse(input);
	free(input);

	send_command(kernelSocket, parsed);

	return EXIT_SUCCESS;
}










t_log* iniciar_logger() {
	return log_create("LFS.log", "LFS", 1, LOG_LEVEL_INFO);
}










t_config* leer_config(){
	return config_create("LFS.config");
}










void levantar_config(Config_final_data *config, t_config* configFile) {
	config->puerto_escucha = config_get_string_value(configFile, "PUERTO_ESCUCHA");
	config->punto_montaje = config_get_string_value(configFile, "PUNTO_MONTAJE");
	config->tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	free(configFile);
}










void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "PUERTO_ESCUCHA=%s", config->puerto_escucha);
	log_info(logger_visible, "PUNTO_MONTAJE=%s", config->punto_montaje);
	log_info(logger_visible, "TAMANIO_VALUE=%s", config->tamanio_value);

	//printf("PUERTO_ESCUCHA=%s\nPUNTO_MONTAJE=%s\nTAMANIO_VALUE=%s\n", config->puerto_escucha, config->punto_montaje,config->tamanio_value);
}

