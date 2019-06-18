#ifndef SERIALIZACION_SERIALIZACION_H_
#define SERIALIZACION_SERIALIZACION_H_

#define RECV_FAIL(msg) {retorno.Argumentos.ERROR.mensajeError = string_from_format(msg);\
						retorno.TipoDeMensaje = ERROR;\
						retorno.opCode = 00000000;\
						return retorno;}

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
#include "../colores/colores.h"
#include "../consistencias/consistencias.h"
#include "../random/random_numbers.h"


typedef struct{
	enum{
		TEXTO_PLANO,
		COMANDO,
		REGISTRO,
		ERROR,
		GOSSIPING_REQUEST,
		GOSSIPING_REQUEST_KERNEL,
		DESCRIBE_REQUEST
	}TipoDeMensaje;
	id opCode; //Nota: para que funcione, el envio y recepcion de mensajes tiene que reutilizar la variable Operacion, o copiar en memoria el codigo para enviarlo en la operacion resultante
	union{
		struct{
			char *texto;
		}TEXTO_PLANO;
		struct{
			char* comandoParseable;
		}COMANDO;
		struct{
			timestamp_t timestamp;
			uint16_t key;
			char* value;
		}REGISTRO;
		struct{
			char* mensajeError;
		}ERROR;
		struct{
			int fin; //Por default en 0 (falso), indica si es el fin de archivo. Poner en 1 cuando se mande la ultima request
			int numeroMemoria;
			char *ipypuerto;
		}GOSSIPING_REQUEST;
		struct{
		}GOSSIPING_REQUEST_KERNEL;
		struct{
			int fin; //Por default en 0 (falso), indica si es el fin de archivo. Poner en 1 cuando se mande la ultima request
			Consistencia consistencia;
			char *nombreTabla;
		}DESCRIBE_REQUEST;
	}Argumentos;
}Operacion;


//FUNCIONES

	/**
	* @NAME: send_command
	* @DESC: envia un mensaje al socket indicado, seleccionando que tipo de mensaje es.
	* 		 segun el tipo de mensaje va a ser el grado de restricciones, por ejemplo,
	* 		 texto plano no tiene restricciones, un comando tiene la restriccion de que sea
	* 		 valido con parsi_validar()
	* @PARAMS:
	* 		socket - el socket destino
	*/
	int send_msg(int socket, Operacion operacionAEnviar);


	/**
	* @NAME: recv_command
	* @DESC: realiza lo necesario para recibir un mensaje de
	* 		 un socket en particular y devuelve ese mensaje en forma
	* 		 de cadena char*
	* @PARAMS:
	* 		socket - el socket origen (el socket que nos envio el mensaje)
	*/
	Operacion recv_msg(int socket);

	#warning "La funcion destruir_operacion() es obsoleta. Hacer free manual"
	/**
	* @NAME: destruir_operacion
	* @DESC:
	* @PARAMS:
	* 		operacion
	*/
	void destruir_operacion(Operacion operacion);



#endif /* SERIALIZACION_SERIALIZACION_H_ */
