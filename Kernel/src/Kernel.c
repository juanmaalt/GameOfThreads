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
	t_log* logger_visible = iniciar_logger(true);
	t_log* logger_invisible=iniciar_logger(false);

	//Funciones .config
	t_config* configFile = leer_config();
	get_data_config(&config, configFile);
	ver_config(&config, logger_visible);


	//Funciones de conexión - Funcion declarada abajo para no llenar el main
	printf("Antes de obtener socket memoria\n");
	printf("IP=%s\nPORT=%s\n",config.ip_memoria, config.puerto_memoria);
	int memoriaSocket = connect_to_server(config.ip_memoria, config.puerto_memoria);
	printf("Antes de funcion_conexion\n");
	funcion_conexion(memoriaSocket);
	log_info(logger_invisible, "Mensaje enviado a la memoria.");

	config_destroy(configFile);
	return 0;

}
/*
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
*/





int funcion_conexion(int socket){
	printf("En función_conexion\n");

	if(socket == EXIT_FAILURE){
		fprintf(stderr, "No se pudo conectar a la memoria\n");
		return EXIT_FAILURE;
	}

	char* input = readline("Escribe un comando para la memoria> ");
	Comando comando_parseado = parse(input);

	if (parsi_validar(comando_parseado) == EXIT_FAILURE)
		return EXIT_FAILURE;

	free(input);

	send_command(socket, comando_parseado);
	Comando *resultado = recv_command(socket);
	parsi_mostrar(*resultado);
	return 0;
}





t_log* iniciar_logger(bool visible) {
	return log_create("Kernel.log", "Kernel", visible, LOG_LEVEL_INFO);
}





t_config* leer_config(){
	return config_create("Kernel.config");
}





void get_data_config(Config_final_data *config, t_config* configFile) {
	config->ip_memoria = config_get_string_value(configFile, "IP_MEMORIA");
	config->puerto_memoria = config_get_string_value(configFile, "PUERTO_MEMORIA");
	config->multiprocesamiento = config_get_string_value(configFile, "MULTIPROCESAMIENTO");
}





void ver_config(Config_final_data *config, t_log* logger_visible){
	log_info(logger_visible, "IP_MEMORIA=%s", config->ip_memoria);
	log_info(logger_visible, "PUERTO_MEMORIA=%s", config->puerto_memoria);
	log_info(logger_visible, "MULTIPROCESAMIENTO=%s", config->multiprocesamiento);
}
