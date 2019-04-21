#ifndef CLIENT_H_
#define CLIENT_H_

//INCLUDES
#include <stdio.h> // Por dependencia de readline en algunas distros de linux
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include "../serializacion/serializacion.h"

//FUNCIONES

	/**
	* @NAME: connect_to_server
	* @DESC: configura una conexion a la ip y el puerto
	* 		 seleccionados.
	* @PARAMS:
	* 		ip - ip del servidor al que queremos conectarnos.
	* 		port - puerto del servidor al que queremos conectarnos.
	*/
	int connect_to_server(char *ip, char *port);

#endif /* CLIENT_H_ */
