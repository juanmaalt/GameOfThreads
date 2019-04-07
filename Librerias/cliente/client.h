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
int conect_to_server(char *ip, char *port);

#endif /* CLIENT_H_ */
