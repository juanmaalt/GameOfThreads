/*
 * Consola.c
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#include "Consola.h"

void *recibir_comandos(void *null){
	pthread_detach(pthread_self());

	printf("Esperando recibir comandos\n");

	for(;;){
		char *userInput = readline("> ");
		ejecutarOperacion(userInput);
	    free(userInput);
	}
	return NULL;
}

