/*
 * Consola.c
 *
 *  Created on: 14 may. 2019
 *      Author: fdalmaup
 */

#include "Consola.h"
void mostrarRetorno(Operacion retorno) {
	switch (retorno.TipoDeMensaje) {
	case REGISTRO:
		log_info(logger_visible,"Timestamp: %llu\nKey:%d\nValue: %s\n",
				retorno.Argumentos.REGISTRO.timestamp,
				retorno.Argumentos.REGISTRO.key,
				retorno.Argumentos.REGISTRO.value);
		return;
	case TEXTO_PLANO:
		log_info(logger_visible,"Resultado: %s\n",retorno.Argumentos.TEXTO_PLANO.texto);
		return;
	case ERROR:
		log_error(logger_error,"%s \n",retorno.Argumentos.ERROR.mensajeError);
		return;
	case COMANDO:
		return;
	}
}

void *recibir_comandos(void *null) {
	pthread_detach(pthread_self());
	Operacion retorno;
	for (;;) {
		char *userInput = readline("> ");
		log_info(logger_invisible,"Request recibida por CONSOLA: %s",userInput);
		retorno = ejecutarOperacion(userInput, true); //libera el userInput en la funcion
		mostrarRetorno(retorno);
		destruir_operacion(retorno);
	}
	return NULL;
}

