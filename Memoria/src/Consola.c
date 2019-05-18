/*
 * Consola.c
 *
 *  Created on: 14 may. 2019
 *      Author: fdalmaup
 */

#include "Consola.h"

void *recibir_comandos(void *null){
	//pthread_detach(pthread_self());


	for(;;){
		char *userInput = readline("> ");
		ejecutarOperacion(userInput);
	    free(userInput);
	}
	return NULL;
}

