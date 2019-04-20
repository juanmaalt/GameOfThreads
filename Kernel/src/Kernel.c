/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"


int main(void) {
	Config_final_data config;

	//Funciones .log
	t_log* logger_visible = iniciar_logger();

	//Funciones .config
	t_config* configFile = leer_config();
	get_data_config(&config, configFile);
	ver_config(&config, logger_visible);
	config_destroy(configFile);

	int miSocket = enable_server("127.0.0.1", "8001");
	if(miSocket == EXIT_FAILURE){
		printf("Error\n");
		return EXIT_FAILURE;
	}

	waiting_connections(miSocket);

	return 0;

}

extern int serve_client(int socketCliente){
	Comando *resultado = recv_command(socketCliente);

	//Es importante realizar este chequeo devolviendo EXIT_FAILURE
	if(resultado == NULL){
		return EXIT_FAILURE;
	}

	printf("Hemos recibido algo!\n");
	parsi_mostrar(*resultado);
	free(resultado);

	return EXIT_SUCCESS;
}











t_log* iniciar_logger() {
	return log_create("Kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
}










t_config* leer_config(){
	return config_create("Kernel.config");
}










void get_data_config(Config_final_data *config, t_config* configFile) {
	config->ip_memoria = config_get_string_value(configFile, "IP_MEMORIA");
	config->puerto_memoria = config_get_string_value(configFile, "PUERTO_MEMORIA");
	config->multiprocesamiento = config_get_string_value(configFile, "MULTIPROCESAMIENTO");
	//free(configFile);
}










void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "IP_MEMORIA=%s", config->ip_memoria);
	log_info(logger_visible, "PUERTO_MEMORIA=%s", config->puerto_memoria);
	log_info(logger_visible, "MULTIPROCESAMIENTO=%s", config->multiprocesamiento);
}

