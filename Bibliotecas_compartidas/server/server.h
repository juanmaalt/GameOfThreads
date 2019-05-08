#ifndef SERVER_H_
#define SERVER_H_

//INCLUDES
#include <stdio.h> // Por dependencia de readline en algunas distros de linux
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <pthread.h>
#include <semaphore.h> //Para semaforos que no son los que trae la biblioteca de pthread
#include "../serializacion/serializacion.h"


//FUNCIONES
	/**
	* @NAME: enable_server
	* @DESC: configura un servidor con la ip seleccionada escuchando
	* 		 al puerto especificado. Devuelve el socket de nuestro servidor
	* @PARAMS:
	* 		ip - ip que va a tener nuestro servidor.
	* 		port - puerto que va a tener nuestro servidor
	*/
	int enable_server(char *ip, char* port);

	/**
	* @NAME: waiting_connections
	* @DESC: configura un servidor con la ip seleccionada escuchando
	* 		 al puerto especificado
	* @PARAMS:
	* 		socketVar - el socket de nuestro servidor
	*/
	void *waiting_connections(int socketVar);




#endif /* SERVER_H_ */
