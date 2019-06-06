/*
 * Consola.c
 *
 *  Created on: 14 may. 2019
 *      Author: fdalmaup
 */

#include "Consola.h"

void *recibir_comandos(void *null){
	pthread_detach(pthread_self());
	Operacion retorno;
	for(;;){
		char *userInput = readline("> ");
		retorno = ejecutarOperacion(userInput);
		//mostrar Retorno
	    free(userInput);
	}
	return NULL;
}

