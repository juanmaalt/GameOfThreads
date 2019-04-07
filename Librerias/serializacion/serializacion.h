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

//ESTRUCTURAS
enum {
	get=0,
	set=1,
	store=2,
	planificador=3,
	coordinador=4,
	esi=5,
	instancia=6,
	solicitud_ejecucion=7, //A priori va a ser usado por la esi
	aceptar_solicitud=8, //Aceptar solicitud de algun pedido
	denegar_solicitud=10, //Deniega temporalmente el pedido
	kill=11, //Cancela totalmente el pedido
	ejecutado=12, //Una instruccion pudo ser ejecutada
	bloqueado=13, //Un proceso entro en bloqueo por una instruccion que lo bloqueo
	error_de_ejecucion=14, //una instruccion no pudo ser ejecutada
	fin_esi=15,
	EL=16,
	LSU=17,
	KE=18
 };


typedef struct{ //Esta es la estructura que devuelve la funcion recv_msg del sistema
	int *id;
	char *clave;
	char *valor;
}Instruccion;

//FUNCIONES
int send_msg_test(int socket); //Funciones para testear el envio de mensajes en tiempo de ejecucion
char *prepare_msg(void);
int recv_msg_test(int socket);
int send_msg(int socket, int id, char *clave, char *valor); //Funcion para el envio de mensajes del sistema
Instruccion *recv_msg(int socket);
void exit_error(int socket, char *msg, void *buffer);

#endif /* SERIALIZACION_SERIALIZACION_H_ */
