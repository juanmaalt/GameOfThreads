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
    int socket = *(int*)nSocket;
    TipoDeMensaje tipo;
    char *resultado = recv_msg(socket, &tipo);

    //Es importante realizar este chequeo devolviendo EXIT_FAILURE
	if(resultado == NULL){
		return NULL;
	}

	printf("Hemos recibido algo!\n");

	if(tipo == COMANDO)
		comando_mostrar(parsear_comando(resultado));
	if(tipo == TEXTO_PLANO)
		printf("%s\n", resultado);


	//Podríamos meter un counter y que cada X mensajes recibidos corra el gossiping
	send_msg(socket, COMANDO, resultado);

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

	int lfsSocket = conectarLFS(&config, logger_invisible);
	int	tamanio_value = handshakeLFS(lfsSocket);
	printf("TAMAÑO_VALUE= %d\n", tamanio_value);


	//Habilita el server y queda en modo en listen / * Inicializar la memoria principal
	int miSocket = enable_server(config.ip, config.puerto);
	log_info(logger_invisible, "Servidor encendido, esperando conexiones");
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




int conectarLFS(Config_final_data *config, t_log* logger_invisible){
	//Obtiene el socket por el cual se va a conectar al LFS como cliente / * Conectarse al proceso File System
	int socket = connect_to_server(config->ip_fileSystem, config->puerto_fileSystem);
	if(socket == EXIT_FAILURE){
		log_error(logger_invisible, "El LFS no está levantado. Cerrar la Memoria, levantar el LFS y volver a levantar la Memoria");
		return EXIT_FAILURE;
	}
	log_error(logger_invisible, "Conectado al LFS. Iniciando Handshake.");

	return socket;
}





int handshakeLFS(int socketLFS){
	send_msg(socketLFS, TEXTO_PLANO, "handshake");

    TipoDeMensaje tipo;
    char *tamanio = recv_msg(socketLFS, &tipo);

	if(tipo == COMANDO)
		printf("Handshake falló. No se recibió el tamaño del value.\n");
	if(tipo == TEXTO_PLANO)
		printf("Handshake exitoso. Se recibió el tamaño del value, es: %d\n", *tamanio);



	return *tamanio;
}
