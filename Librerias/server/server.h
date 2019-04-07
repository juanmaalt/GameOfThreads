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
int enable_server(char *port, char* iPAddress);
void *waiting_conections(void *socketVar);

//GLOBALES
int socketEsi;
Instruccion *respuesta;



#endif /* SERVER_H_ */
