/*
 * Memoria.h
 *
 *  Created on: 13 abr. 2019
 *      Author:	fdalmaup
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#define STANDARD_PATH_MEMORIA_CONFIG "Memoria.config"

#define RETURN_ERROR(mensaje) {if(logger_error != NULL && logger_invisible != NULL){ \
									log_error(logger_error, "%s", mensaje); \
									log_error(logger_invisible, "%s", mensaje); \
								}else{ \
									printf(RED"%s"STD"\n", mensaje); \
								} \
								return EXIT_FAILURE; \
							   }

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <colores/colores.h>
#include <commons/collections/queue.h>
#include <readline/readline.h>
#include <cliente/cliente.h>
#include <parser/parser_comando.h>
#include <server_multithread/server_multithread.h>
#include <serializacion/serializacion.h>
#include <epoch/epoch.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h> //Para mkdir

#include "Consola.h"
#include "API/APIMemoria.h"
#include "RutinasDeLiberacion.h"
#include "Gossiping.h"
#include "ManejoDeMemoria.h"
#include "ComunicacionFS.h"


#define ERROR_MEMORIA_FULL -1 //TODO: PUEDE DESAPARECER o dejar como salvaguarda

//ESTRUCTURAS
struct Config_datos_fijos{
	char *ip;
	char *puerto;
	char *ip_fileSystem;
	char *puerto_fileSystem;
	char *ip_seeds;
	char *puerto_seeds;
	char *tamanio_memoria;
	char *numero_memoria;
};

typedef struct Config_datos_fijos fConfig;

struct Config_datos_variables{
	int (*retardoMemoria)();
	int (*retardoFS)();
	int (*retardoJOURNAL)();
	int (*retardoGossiping)();
}; //Se actualizan en tiempo de ejecucion

typedef struct Config_datos_variables vConfig;

sem_t journal;
pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexTablaSegmentos;

int configuracion_inicial(void);
int realizarHandshake(void);
void *connection_handler(void *);

t_log* iniciar_logger(char* fileName, bool visibilidad, t_log_level level);
void loggearRetorno(Operacion, t_log*);
int inicializar_configs();


void extraer_data_fija_config(void);
void mostrar_por_pantalla_config();


int extraer_retardo_memoria();
int extraer_retardo_FS();
int extraer_retardo_JOURNAL();
int extraer_retardo_Gossiping();

//GLOBALES

/*Las de log y config son globales para poder acceder a ellos desde cualquier lado, pudiendo leer del config en tiempo de ejecucion y escribir en los logs sin
* pasarlos por parametro
*/

t_config* configFile;

fConfig fconfig; //Contiene solo los datos fijos del config
vConfig vconfig; //Contiene solo los datos variables del config


t_log* logger_visible;
t_log* logger_invisible;
t_log* logger_error;

//Estructuras de memoria

typedef unsigned long long timestamp_t;
/*
typedef struct marco{
	uint16_t key;
	timestamp_t timestamp;
	char* value;
}marco_t;  		// puede tener un __attribute__((packed, aligned(1))) para evitar el padding
*/

//typedef void pagina_t;

typedef struct registroTablaPag{
	timestamp_t ultimoUso;		//Para LRU
	bool flagModificado;
	int nroPagina;
	int nroMarco; //coincide con el numero de marco en memoria

}registroTablaPag_t;

typedef struct tabla_de_paginas{
	t_list* registrosPag;
}tabla_de_paginas_t;

typedef struct segmento{
	char* pathTabla;
	tabla_de_paginas_t* tablaPaginas;
}segmento_t;


typedef struct{
	t_list* listaSegmentos;
}tabla_de_segmentos_t;

//GLOBALES
tabla_de_segmentos_t tablaSegmentos;

//MarcoCtrlBlock

typedef struct MCB{
	int nroMarco;

	//Faltan cosas
}MCB_t;

//Bloque de Memoria

typedef struct{
	void* memoria;	//Bloque de memoria
	int tamanioMarco;
	int cantidadMarcos;
	t_queue *marcosLibres;
	t_list* listaAdminMarcos;
}memoria_principal;

//GLOBALES
memoria_principal memoriaPrincipal;

pthread_t idConsola;
pthread_t idJournal;

//TODO: HILO DE GOSSIPING?

//socket de FILE SYSTEM TODO: A ELIMINAR
int lfsSocket;

//socket de la Memoria
int memorySocket;

//FUNCIONES

int threadConnection(int serverSocket, void *funcionThread);

int inicializar_memoriaPrincipal();

void memoriaConUnSegmentoYUnaPagina(void);

int iniciar_consola();
int iniciar_serverMemoria (void);
int iniciar_Journal(void);
void *realizarJournal(void*);

#endif /* MEMORIA_H_ */
