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

	/**
	* @NAME: serve_client
	* @DESC: se debe definir en el codigo propio como extern. Se define aca,
	* 		 se implementa en otro archivo. Define el comportamiento que
	* 		 debera tener 'waiting_connections' ante un determinado cliente.
	* 		 Es un modelo sincronizado, por lo tanto, cuando se ejecute esta
	* 		 funcion va a ser por que un cliente nos mando algo y nosotros
	* 		 estamos en condiciones de recibirlo. Eventualmente podemos
	* 		 mandarle algo nosotros tambien. Por ultimo, es requisito que
	* 		 antes se haya llamado a 'waiting_connections'. Aunque la podemos
	* 		 llamar nosotros si conocemos el socket del cliente. Es poco probable
	* 		 que eso pase.
	* @PARAMS:
	* 		socketCliente - el socket del cliente que vamos a atender
	* @IMPORTANTE: Si hay algun error en la funcion para recibir (devuelve
	* 			   un resultado menor o igual a 0), el retorno de serve_cliente
	* 			   debe ser EXIT_FAILURE para garantizar que waiting_connections
	* 			   siga andando bien.
	*/
	int serve_client(int socketCliente);




#endif /* SERVER_H_ */
