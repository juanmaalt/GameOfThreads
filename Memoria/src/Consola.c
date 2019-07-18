/*
 * Consola.c
 *
 *  Created on: 14 may. 2019
 *      Author: fdalmaup
 */

#include "Consola.h"


void *recibir_comandos(void *null) {
	pthread_detach(pthread_self());
	Operacion retorno;
	for (;;) {
		char *userInput = readline("> ");
		log_info(logger_invisible,"Consola.c: recibir_comandos: Request recibida por CONSOLA: %s",userInput);
		retorno = ejecutarOperacion(userInput, true); //libera el userInput en la funcion
		loggearRetorno(retorno, logger_visible);
		destruir_operacion(retorno);
	}
	return NULL;
}

