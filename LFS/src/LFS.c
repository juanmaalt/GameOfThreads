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
		//if(strcmp(resultado, "handshake"))
			handshakeMemoria(socket);
		else
			printf("No se pudo conectar la Memoria\n");

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
	//Meter funcion para levantar las variables de tiempo retardo y tiempo_dump

	//Habilita el server y queda en modo en listen / * Inicializar la File System principal
	int miSocket = enable_server(config.ip, config.puerto_escucha);
	log_info(logger_invisible, "Servidor encendido, esperando conexiones");
	threadConnection(miSocket, connection_handler);



	/*
	switch(comando_parseado.keyword){
		case SELECT:
			selectAPI(comando_parseado);
			log_info(logger_invisible, "%s",input);

			break;
		case INSERT:
			insertAPI(comando_parseado);
			log_info(logger_invisible, "%s",input);

			break;
		case CREATE:
			createAPI(comando_parseado);
			log_info(logger_invisible, "%s",input);


			break;
		case DESCRIBE:
			describeAPI(comando_parseado);
			log_info(logger_invisible, "%s",input);

			break;
		case DROP:
			dropAPI(comando_parseado);
			log_info(logger_invisible, "%s",input);

			break;
		default:
			return EXIT_FAILURE;
	}
	*/

	config_destroy(configFile);
	return EXIT_SUCCESS;
}



void selectAPI(Comando comando){

}
void insertAPI(Comando comando){


}
void createAPI(Comando comando){


}
void describeAPI(Comando comando){

}
void dropAPI(Comando comando){

}





t_log* iniciar_logger(bool visible) {
	return log_create("LFS.log", "LFS", visible, LOG_LEVEL_INFO);
}





t_config* leer_config(){
	return config_create("LFS.config");
}





void get_data_config(Config_final_data *config, t_config* configFile) {
	config->ip = config_get_string_value(configFile, "IP");
	config->puerto_escucha = config_get_string_value(configFile, "PUERTO_ESCUCHA");
	config->punto_montaje = config_get_string_value(configFile, "PUNTO_MONTAJE");
	//config->retardo = config_get_string_value(configFile, "RETARDO");
	config->tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	//config->tiempo_dump = config_get_string_value(configFile, "TIEMPO_DUMP");
	//free(configFile);
}




void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "IP=%s", config->ip);
	log_info(logger_visible, "PUERTO_ESCUCHA=%s", config->puerto_escucha);
	log_info(logger_visible, "PUNTO_MONTAJE=%s", config->punto_montaje);
	//log_info(logger_visible, "RETARDO=%s", config->retardo);
	log_info(logger_visible, "TAMANIO_VALUE=%s", config->tamanio_value);
	//log_info(logger_visible, "TIEMPO_DUMP=%s", config->tiempo_dump);
}




void handshakeMemoria(int socketMemoria){
	printf("Se conectó la Memoria\n");
	char* tamanio = "500";

	send_msg(socketMemoria, TEXTO_PLANO, tamanio);
}
