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
	//t_config* config = leer_config();

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

t_config* leer_config() {
	return config_create("Kernel.config");
}
