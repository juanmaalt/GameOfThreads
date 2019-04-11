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
	levantar_config();
	ver_config();

	int kernelSocket = conect_to_server("127.0.0.1", "8002");

	Comando parsed = parse("DESCRIBE nombreTabla");

	send_comando(kernelSocket, parsed);

	return EXIT_SUCCESS;
}


void levantar_config() {
	t_config *configFile = config_create("LFS.config");

	config.puerto_escucha = config_get_string_value(configFile, "PUERTO_ESCUCHA");
	config.punto_montaje = config_get_string_value(configFile, "PUNTO_MONTAJE");
	config.retardo = config_get_string_value(configFile, "RETARDO");
	config.tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	config.tiempo_dump = config_get_string_value(configFile, "TIEMPO_DUMP");

	//free(configFile);
}

void ver_config(void){
	printf("PUERTO_ESCUCHA=%s\nPUNTO_MONTAJE=%s\nRETARDO=%s\nTAMANIO_VALUE=%s\nTIEMPO_DUMP=%s\n", config.puerto_escucha, config.punto_montaje, config.retardo, config.tamanio_value, config.tiempo_dump);
}
