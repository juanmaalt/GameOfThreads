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
	t_log* logger_visible = iniciar_logger(true);
	t_log* logger_invisible=iniciar_logger(false);

	//Funciones .config
	t_config* configFile = leer_config();
	get_data_config(&config, configFile);
	ver_config(&config, logger_visible);
	//Meter funcion para levantar las variables de tiempo retardo y tiempo_dump
	config_destroy(configFile);



	int kernelSocket = connect_to_server("127.0.0.1", "8005");

	if(kernelSocket == EXIT_FAILURE){
		fprintf(stderr, "No se pudo conectar al server\n");
		return EXIT_FAILURE;
	}

	char* input = readline("Mandele un comando al kernel> ");
	Comando comando_parseado = parse(input);




	if (parsi_validar(comando_parseado) == EXIT_FAILURE)
		return EXIT_FAILURE;

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
	free(input);
	send_command(kernelSocket, comando_parseado);

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
	config->puerto_escucha = config_get_string_value(configFile, "PUERTO_ESCUCHA");
	config->punto_montaje = config_get_string_value(configFile, "PUNTO_MONTAJE");
	config->tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	//free(configFile);
}










void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "PUERTO_ESCUCHA=%s", config->puerto_escucha);
	log_info(logger_visible, "PUNTO_MONTAJE=%s", config->punto_montaje);
	log_info(logger_visible, "TAMANIO_VALUE=%s", config->tamanio_value);
}

