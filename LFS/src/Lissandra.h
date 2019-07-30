/*
 * LFS.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef LISSANDRA_H_
#define LISSANDRA_H_
#define STANDARD_PATH_LFS_CONFIG "LFS.config"
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
#include <libgen.h>

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <commons/txt.h>

#include <cliente/cliente.h>
#include <parser/parser_comando.h>
#include <server_multithread/server_multithread.h>
#include <epoch/epoch.h>
#include <colores/colores.h>

#include <readline/readline.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/inotify.h>

#include "adhoc/Consola.h"
#include "adhoc/APILissandra.h"
#include "adhoc/Bitmap.h"
#include "FileSystem.h"
#include "Compactador.h"


/*ESTRUCTURAS*/
typedef struct{
	char *ip;
	char *puerto_escucha;
	char *ip_memoria;
	char *puerto_memoria;
	char *punto_montaje;
	char *tamanio_value;
}Config_final_data;

struct Config_datos_variables{
	int retardo;
	int tiempoDump;
}; //Se actualizan en tiempo de ejecucion

typedef struct Config_datos_variables vConfig;

typedef unsigned long long timestamp_t;

typedef struct registro_t{
	timestamp_t timestamp;
	uint16_t key;
	char* value;
}Registro;

typedef struct semt_t{
	char* tabla;
	int peticionesEnEspera;
	int peticionesEjecutando;
	int peticionesEnEsperaSelect;
	int peticionesEjecutandoSelect;
	sem_t semaforoGral;
	sem_t semaforoSelect;
	sem_t enEjecucion;
	sem_t enEjecucionSelect;
	pthread_t compactacionService;
	timestamp_t inicioBloqueo;
	timestamp_t finBloqueo;
}SemaforoTabla;

/*GLOBALES*/
t_log* logger_visible;
t_log* logger_invisible;
t_log* logger_error;
t_config* configFile;
Config_final_data config;
vConfig vconfig;
int miSocket;

int socketMemoria;

pthread_t idConsola;

t_dictionary* memtable;
t_dictionary* dPeticionesPorTabla;
t_list* semaforosPorTabla;
t_bitarray* bitarray;
char* bitmap;

sem_t mutexPeticionesPorTabla;
sem_t dumpTest;

pthread_t inotify;

/*FUNCIONES*/
/*CONNECTION HANDLER*/
void *connection_handler(void *nSocket);
/*FUNCIONES VARIAS*/
int configuracion_inicial();
t_log* iniciar_logger(bool visible, char* path);
t_config* leer_config();
void extraer_data_config();
void refrescar_vconfig();
void ver_config();
t_dictionary* inicializarDiccionario();
void handshakeMemoria(int socket);
int threadConnection(int serverSocket, void *funcionThread);
int iniciar_consola();
Operacion ejecutarOperacion(char*);
void encolar_peticion(char *tabla, char* peticion);

/*FUNCIONES BOOL*/
uint16_t obtenerKey(Registro* registro);
timestamp_t obtenerTimestamp(Registro* registro);

/*FUNCIONES SEEK*/
Registro* fseekBloque(int key, char* listaDeBloques);

/*FUNCIONES FINALIZACION*/
void rutinas_de_finalizacion();

/*FUNCIONES TEST*/
void agregarDatos(t_dictionary* memtable);

/*FUNCIONES DUMP*/
void setPathTabla(char* path, char* nombreTabla);
void* dump();
void dumpRegistro(FILE* file, Registro* registro);
void dumpTabla(char* nombreTable, void* value);

#endif /* LISSANDRA_H_ */

