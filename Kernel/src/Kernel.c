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
	waiting_connections(miSocket);

	return 0;

}

extern int serve_client(int socketCliente){
	printf("asdasd\n");
	return EXIT_SUCCESS;
}

t_config* leer_config() {
	return config_create("Kernel.config");
}
