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


void *connection_handler(void *nSocket){
    printf("Esperando conexion\n");
    int socket = *(int*)nSocket;
    TipoDeMensaje tipo;
    char *resultado = recv_command(socket, &tipo);

    //Es importante realizar este chequeo devolviendo EXIT_FAILURE
	if(resultado == NULL){
		return NULL;
	}

	printf("Hemos recibido algo!\n");

	if(tipo == COMANDO)
		parsi_mostrar(parse(resultado));
	if(tipo == TEXTO_PLANO)
		printf("%s\n", resultado);


	//Podríamos meter un counter y que cada X mensajes recibidos corra el gossiping


	if(resultado != NULL)
		free(resultado);

	return NULL;
}





int main(void) {
	Config_final_data config;

	//Funciones .log
	t_log* logger_visible = iniciar_logger(true);
	t_log* logger_invisible=iniciar_logger(false);

	//Funciones .config
	t_config* configFile = leer_config();
	get_data_config(&config, configFile);
	ver_config(&config, logger_visible);


	/*Conectarse al proceso File System y realizar handshake necesario para obtener los datos requeridos.
		 Esto incluye el tamaño máximo del Value configurado para la administración de las páginas.

	 * Inicializar la memoria principal (que se explican en los siguientes apartados).

	 * Iniciar el proceso de Gossiping (explicado en profundidad en el Anexo III) que consiste en la
	   comunicación de cada proceso memoria con otros procesos memorias, o seeds, para intercambiar y
	   descubrir otros procesos memorias que se encuentren dentro del pool (conjunto de memorias).

	*/

	//Obtiene el socket por el cual se va a conectar al LFS como cliente / * Conectarse al proceso File System
/*
	 int lfsSocket = connect_to_server(config.ip_fileSystem, config.puerto_fileSystem);
	 if(lfsSocket == EXIT_FAILURE){
		log_info(logger_visible, "El LFS no está levantado. Cerrar la Memoria, levantar el LFS y volver a levantar la Memoria");
		 return EXIT_FAILURE;
	 }
*/


	printf("IP=%s\nPORT=%s\n",config.ip,config.puerto);

	//Habilita el server y queda en modo en listen / * Inicializar la memoria principal
	int miSocket = enable_server(config.ip, config.puerto);
	log_info(logger_invisible, "Servidor encendido, esperando conexiones");
	printf("funcionó el enable\n");
	threadConnection(miSocket, connection_handler);

	config_destroy(configFile);
}





t_log* iniciar_logger(bool visible) {
	return log_create("Memoria.log", "Memoria", visible, LOG_LEVEL_INFO);
}




t_config* leer_config() {
	return config_create("Memoria.config");
}




void get_data_config(Config_final_data *config, t_config* configFile) {
	config->ip = config_get_string_value(configFile, "IP");
	config->puerto = config_get_string_value(configFile, "PUERTO");
	config->ip_fileSystem = config_get_string_value(configFile, "IP_FS");
	config->puerto_fileSystem = config_get_string_value(configFile, "PUERTO_FS");
	config->ip_seeds = config_get_string_value(configFile, "IP_SEEDS");
	config->puerto_seeds = config_get_string_value(configFile, "PUERTO_SEEDS");
	config->tamanio_memoria = config_get_string_value(configFile, "TAM_MEM");
	config->numero_memoria = config_get_string_value(configFile, "MEMORY_NUMBER");
}





void ver_config(Config_final_data *config, t_log* logger_visible) {
	log_info(logger_visible, "IP=%s", config->ip);
	log_info(logger_visible, "PUERTO=%s", config->puerto);
	log_info(logger_visible, "IP_FS=%s", config->ip_fileSystem);
	log_info(logger_visible, "PUERTO_FS=%s", config->puerto_fileSystem);
	log_info(logger_visible, "IP_SEEDS=%s", config->ip_seeds);
	log_info(logger_visible, "PUERTO_SEEDS=%s", config->puerto_seeds);
	log_info(logger_visible, "TAM_MEM=%s", config->tamanio_memoria);
	log_info(logger_visible, "MEMORY_NUMBER=%s", config->numero_memoria);
}




