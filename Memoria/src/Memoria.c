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

int main(void) {
	Config_final_data config;

	//Funciones .log
	t_log* logger_visible = iniciar_logger();

	//Funciones .config
	t_config* configFile = leer_config();
	get_data_config(&config, configFile);
	ver_config(&config, logger_visible);

	config_destroy(configFile);
}










t_log* iniciar_logger() {
	return log_create("LFS.log", "LFS", 1, LOG_LEVEL_INFO);
}









t_config* leer_config() {
	return config_create("Memoria.config");
}










void get_data_config(Config_final_data *config, t_config* configFile) {
	config->puerto = config_get_string_value(configFile, "PUERTO");
	config->ip_fileSystem = config_get_string_value(configFile, "IP_FS");
	config->puerto_fileSystem = config_get_string_value(configFile,
			"PUERTO_FS");
	config->ip_seeds = config_get_string_value(configFile, "IP_SEEDS");
	config->puerto_seeds = config_get_string_value(configFile, "PUERTO_SEEDS");
	config->tamanio_memoria = config_get_string_value(configFile, "TAM_MEM");
	config->numero_memoria = config_get_string_value(configFile,
			"MEMORY_NUMBER");
}










void ver_config(Config_final_data *config, t_log* logger_visible) {
	log_info(logger_visible, "PUERTO=%s", config->puerto);
	log_info(logger_visible, "IP_FS=%s", config->ip_fileSystem);
	log_info(logger_visible, "PUERTO_FS=%s", config->puerto_fileSystem);
	log_info(logger_visible, "IP_SEEDS=%s", config->ip_seeds);
	log_info(logger_visible, "PUERTO_SEEDS=%s", config->puerto_seeds);
	log_info(logger_visible, "TAM_MEM=%s", config->tamanio_memoria);
	log_info(logger_visible, "MEMORY_NUMBER=%s", config->numero_memoria);
}

