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

	/**
	* @NAME: serve_client
	* @DESC: no usar. Solo soluciona un error de linkeo. La funcion solo
	* 		 tiene sentido en contexto de servidor, el unico lugar donde se
	* 		 deberia llamar es waiting_connections. Al declararse como extern,
	* 		 aparentemente las funciones de cliente necesitan una implementacion
	* 		 de la funcion. Caso contrario, tira error de linkeo. En resumen:
	* 		 usar esta funcion solo si sos servidor. Definirla en un cliente no
	* 		 va a hacer nada.
	* @PARAMS:
	*/
	extern int serve_client(int socketCliente);

#endif /* CLIENT_H_ */
