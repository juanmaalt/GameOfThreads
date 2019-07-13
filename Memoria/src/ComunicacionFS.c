/*
 * ComunicacionFS.c
 *
 *  Created on: 24 jun. 2019
 *      Author: fdalmaup
 */

#include "ComunicacionFS.h"

int handshakeLFS(int socketLFS) {
	Operacion handshake;

	handshake.TipoDeMensaje= TEXTO_PLANO;

	handshake.Argumentos.TEXTO_PLANO.texto= string_from_format("handshake");

	send_msg(socketLFS, handshake);

	destruir_operacion(handshake);

	//Recibo el tamanio
	//while((handshake = recv_msg(socketLFS)).TipoDeMensaje)
	handshake = recv_msg(socketLFS);

	switch(handshake.TipoDeMensaje){
		case TEXTO_PLANO:
			tamanioValue=atoi(handshake.Argumentos.TEXTO_PLANO.texto);
			destruir_operacion(handshake);
			break;
		case ERROR:
		case REGISTRO:
		case COMANDO:
		default:
			return EXIT_FAILURE;
	}


	//Pido el punto de montaje
	handshake.TipoDeMensaje= TEXTO_PLANO;
	handshake.Argumentos.TEXTO_PLANO.texto=string_from_format("handshake pathLFS");

	send_msg(socketLFS, handshake);

	destruir_operacion(handshake);

	//Recibo el punto de montaje
	handshake = recv_msg(socketLFS);

	switch(handshake.TipoDeMensaje){
			case TEXTO_PLANO:
				pathLFS=string_from_format(handshake.Argumentos.TEXTO_PLANO.texto);
				destruir_operacion(handshake);
				break;
			case ERROR:
			case REGISTRO:
			case COMANDO:
			default:
				return EXIT_FAILURE;
		}

	log_info(logger_visible, "El size del value es: %d", tamanioValue);
	log_info(logger_visible, "El punto de montaje es: %s",pathLFS);

	return EXIT_SUCCESS;
}

int conectarLFS() {
	//Obtiene el socket por el cual se va a conectar al LFS como cliente / * Conectarse al proceso File System
	int socket = connect_to_server(fconfig.ip_fileSystem, fconfig.puerto_fileSystem);
	if (socket == EXIT_FAILURE) {
		log_error(logger_error,"COMUNICACIONESFS: conectarLFS: El LFS no está levantado. Cerrar la Memoria, levantar el LFS y volver a levantar la Memoria");
		return EXIT_FAILURE;
	}
	return socket;
}

//Para API

void enviarRequestFS(char* input) {
	lfsSocket = conectarLFS(); //TODO: NO DEJARLA COMO GLOBAL

	Operacion request;
	request.TipoDeMensaje = COMANDO;

	request.Argumentos.COMANDO.comandoParseable = string_from_format(input);

	send_msg(lfsSocket, request);

	destruir_operacion(request);
}

Operacion recibirRequestFS(void) {
	Operacion resultado;
	resultado = recv_msg(lfsSocket);
	return resultado;
}


