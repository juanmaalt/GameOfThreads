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
	t_config* configFile = leer_config();

	obtener_data_config(&config, configFile);

	ver_config(&config);

	config_destroy(configFile);
}

t_config* leer_config() {
	return config_create("Memoria.config");
}

void obtener_data_config(Config_final_data *config, t_config* configFile) {
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

void ver_config(Config_final_data *config) {
	printf(
			"PUERTO=%s\nIP_FS=%s\nPUERTO_FS=%s\nIP_SEEDS=%s\nPUERTO_SEEDS=%s\nTAM_MEM=%s\nMEMORY_NUMBER=%s \n",
			config->puerto, config->ip_fileSystem, config->puerto_fileSystem,
			config->ip_seeds, config->puerto_seeds, config->tamanio_memoria,
			config->numero_memoria);
}

