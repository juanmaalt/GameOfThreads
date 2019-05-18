#ifndef SERVER_MULTITHREAD_SERVER_MULTITHREAD_H_
#define SERVER_MULTITHREAD_SERVER_MULTITHREAD_H_

#include <pthread.h> //Para manejo de threads
#include "../server/server.h" //Para enable_server, y en consecuencia lo que incluya server.h
//server_multithread es un caso particular de server.c

	/**
	* @NAME: threadConnection
	* @DESC: se pone a escuchar conexiones por medio de un socket pasado por
	* 		 parametro (socket del server). Por cada conexion entrante, lanza
	* 		 una funcion hecha por el usuario (funcionThread)
	* @PARAMS:
	* 		socket - socket de escucha
	* 		funcionThread - operacion que se va a realizar cada vez que entre una conexion.
	*/
	int threadConnection(int socket, void *funcionThread);

#endif /* SERVER_MULTITHREAD_SERVER_MULTITHREAD_H_ */
