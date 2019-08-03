/*
 * Consola.c
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#include "Consola.h"

void mostrarRetorno(Operacion retorno){
	switch (retorno.TipoDeMensaje) {
	case REGISTRO:
		log_info(logger_visible, "<REGISTRO>\nTimestamp: %llu\nKey:%d\nValue: %s",
				retorno.Argumentos.REGISTRO.timestamp,
				retorno.Argumentos.REGISTRO.key,
				retorno.Argumentos.REGISTRO.value);
		return;
	case TEXTO_PLANO:
		log_info(logger_visible,"<RESULTADO>\n%s",retorno.Argumentos.TEXTO_PLANO.texto);
		return;
	case ERROR:
		log_error(logger_visible,"<ERROR>\n%s",retorno.Argumentos.ERROR.mensajeError);
		return;
	case COMANDO:
		return;
	case DESCRIBE_REQUEST:
		log_info(logger_visible,"<DESCRIBE>\n%s",retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
		return;
	case HANDSHAKE:
		return;
	case GOSSIPING_REQUEST:
		return;
	case GOSSIPING_REQUEST_KERNEL:
		return;
	case ERROR_JOURNAL:
		return;
	case ERROR_MEMORIAFULL:
		return;
	}
}

void *recibir_comandos(void *null){
	pthread_detach(pthread_self());

	Operacion retorno;
	for(;;){
		char *userInput = readline("> ");
		retorno = ejecutarOperacion(userInput);
		//mostrarRetorno(retorno);
		free(userInput);
		destruir_operacion(retorno);
	}
	return NULL;
}




