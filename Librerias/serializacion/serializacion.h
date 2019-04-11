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

char *prepare_msg(void);
int send_comando(int socket, Comando comando); //Funcion para el envio de mensajes del sistema
Comando *recv_comando(int socket);
void exit_error(int socket, char *msg, void *buffer);


#endif /* SERIALIZACION_SERIALIZACION_H_ */
