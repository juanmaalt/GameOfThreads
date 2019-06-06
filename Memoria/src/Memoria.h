/*
 * Memoria.h
 *
 *  Created on: 13 abr. 2019
 *      Author:	fdalmaup
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#define STANDARD_PATH_MEMORIA_CONFIG "Memoria.config"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <readline/readline.h>
#include <cliente/cliente.h>
#include <parser/parser_comando.h>
#include <server_multithread/server_multithread.h>
#include <serializacion/serializacion.h>
#include <epoch/epoch.h>
#include <pthread.h>
#include <semaphore.h>

#include "Consola.h"
#include "APIMemoria.h"

#define RED "\x1b[31m"
#define STD "\x1b[0m"
#define ERROR_MEMORIA_FULL -1

//Para evitar levantar el LFS
int	tamanioValue;
char *pathLFS;


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


int configuracion_inicial(void);
int realizarHandshake(void);
void *connection_handler(void *);

void liberarRecursos();

t_log* iniciar_logger(bool);
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
	int numeroPagina; //coincide con el numero de marco en memoria

	bool flagModificado; //EN REVISION
	int countUso;		//EN REVISION
}registroTablaPag_t;

typedef struct{
	t_list* registrosPag;
}tabla_de_paginas_t;

typedef struct segmento{
	char* pathTabla;
	tabla_de_paginas_t* tablaPaginas;
}segmento_t;

char* obtenerPath(segmento_t* segmento);

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

//Funciones Memoria

//marco_t * agregarMarcoAMemoria(marco_t *);
void mostrarContenidoMemoria(void);

void asignarPathASegmento(segmento_t * , char* );
int colocarPaginaEnMemoria(timestamp_t , uint16_t , char* );
void crearRegistroEnTabla(tabla_de_paginas_t *, int );

//GLOBALES
memoria_principal memoriaPrincipal;

pthread_t idConsola;

//FUNCIONES
int conectarLFS();
int handshakeLFS(int socketLFS);
int threadConnection(int serverSocket, void *funcionThread);

int inicializar_memoriaPrincipal();

void memoriaConUnSegmentoYUnaPagina(void);

int iniciar_consola();



//GOSSIPING

//Memorias conocidas
typedef struct knownMemory {
	int memory_number;
	char *ip ;
	char *ip_port;
}knownMemory_t;

t_list* listaMemoriasConocidas;


pthread_t idGossipSend;
pthread_t idGossipReciv;


char **IPs;
char **IPsPorts;

int iniciar_gossiping();

void liberarIPs(char** );
void quitarCaracteresPpioFin(char* );
int conectarConSeed(char**,char**);
void *conectar_seeds(void*);
void *recibir_seeds(void*);
int ConsultoPorMemoriasConocidas(int );



#endif /* MEMORIA_H_ */
