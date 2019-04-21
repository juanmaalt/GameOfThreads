#ifndef SERIALIZACION_SERIALIZACION_H_
#define SERIALIZACION_SERIALIZACION_H_

//INCLUDES
#include <stdio.h> // Por dependencia de readline en algunas distros de linux
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <readline/readline.h> // Para usar readline. Asi como se agregan las commons agregar "readline" like by -lcommons
#include <commons/log.h> // Para logear
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <stdbool.h> ////Para bool que se usan en la funcion log (relativamente opcional)
#include "../parser/parsi_mod.h" //Para poder enviar estructuras de tipo Comando


//FUNCIONES

	/**
	* @NAME: send_command
	* @DESC: envia un dato de tipo comando (ver parser) al socket
	* 		 indicado
	* @PARAMS:
	* 		socket - el socket destino
	* 		comando - el mensaje a enviar
	*/
	int send_command(int socket, Comando comando);

	/**
	* @NAME: recv_command
	* @DESC: realiza todo lo necesario para recibir un mensaje de
	* 		 un socket en particular y devuelve ese mensaje en forma
	* 		 de comando
	* @PARAMS:
	* 		socket - el socket origen (el socket que nos envio el mensaje)
	*/
	Comando *recv_command(int socket);


#endif /* SERIALIZACION_SERIALIZACION_H_ */
