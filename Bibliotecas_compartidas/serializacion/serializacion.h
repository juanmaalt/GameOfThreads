#ifndef SERIALIZACION_SERIALIZACION_H_
#define SERIALIZACION_SERIALIZACION_H_
#include "../colores/colores.h"

//INCLUDES
#include <stdio.h> // Por dependencia de readline en algunas distros de linux
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <readline/readline.h> // Para usar readline. Asi como se agregan las commons agregar "readline" like by -lcommons
#include <commons/log.h> // Para logear
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <stdbool.h> ////Para bool que se usan en la funcion log (relativamente opcional)

#include "../epoch/epoch.h"
#include "../parser/parser_comando.h" //Para poder enviar estructuras de tipo Comando


//ENUM
typedef enum{
	TEXTO_PLANO, 	//char*
	COMANDO,		//REQUEST
	REGISTRO,  	    // IMPLICA TIMESTAMP, KEY Y VALUE EN ESE ORDEN
	ERROR,			//char* con detalle de error

}TipoDeMensaje;

typedef struct{
	TipoDeMensaje codigoOperacion;
	char *mensaje; //detalle del mensaje (para ERROR)
	timestamp_t timestamp;
	uint16_t key;
	char* value;	//value del registro en una key determinada
}resultado;


//FUNCIONES

	/**
	* @NAME: send_command
	* @DESC: envia un mensaje al socket indicado, seleccionando que tipo de mensaje es.
	* 		 segun el tipo de mensaje va a ser el grado de restricciones, por ejemplo,
	* 		 texto plano no tiene restricciones, un comando tiene la restriccion de que sea
	* 		 valido con parsi_validar()
	* @PARAMS:
	* 		socket - el socket destino
	* 		tipo - tipo de mensaje a enviar
	* 		mensaje - mensaje a enviar
	*/
	int send_msg(int socket, TipoDeMensaje tipo, char* mensaje);


	/**
	* @NAME: recv_command
	* @DESC: realiza lo necesario para recibir un mensaje de
	* 		 un socket en particular y devuelve ese mensaje en forma
	* 		 de cadena char*
	* @PARAMS:
	* 		socket - el socket origen (el socket que nos envio el mensaje)
	* 		tipo - puntero usado para recibir el tipo de mensaje, liberarlo en el codigo propio.
	*/
	char *recv_msg(int socket, TipoDeMensaje *tipo);


#endif /* SERIALIZACION_SERIALIZACION_H_ */
