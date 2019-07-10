/*
 * LFS.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef LISSANDRA_H_
#define LISSANDRA_H_
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
	char *retardo; //Esta config la tengo que dejar como variable.
	char *tamanio_value;
	char *tiempo_dump; //Esta config la dejamos como variable.
}Config_final_data;

typedef unsigned long long timestamp_t;

typedef struct{
	timestamp_t timestamp;
	uint16_t key;
	char* value;
}Registro;


/*GLOBALES*/
t_log* logger_visible;
t_log* logger_invisible;
t_config* configFile;
Config_final_data config;
int socketMemoria;

pthread_t idConsola;

t_dictionary* memtable;
t_dictionary* diccCompactacion;
t_bitarray* bitarray;


/*FUNCIONES*/
void *connection_handler(void *nSocket);

int configuracion_inicial();
t_log* iniciar_logger(bool);
t_config* leer_config();
void extraer_data_config();
void ver_config();
t_dictionary* inicializarDiccionario();
void handshakeMemoria(int socket);
int threadConnection(int serverSocket, void *funcionThread);
int iniciar_consola();
Operacion ejecutarOperacion(char*);

void agregarDatos(t_dictionary* memtable);

uint16_t obtenerKey(Registro* registro);
timestamp_t obtenerTimestamp(Registro* registro);


/*FUNCIONES DUMP*/
void setPathTabla(char* path, char* nombreTabla);
void dump(t_dictionary* memtable);
void dumpRegistro(FILE* file, Registro* registro);
void dumpTabla(char* nombreTable);


#endif /* LISSANDRA_H_ */

