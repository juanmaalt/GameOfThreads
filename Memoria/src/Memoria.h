/*
 * Memoria.h
 *
 *  Created on: 13 abr. 2019
 *      Author:	fdalmaup
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <cliente/cliente.h>
#include <parser/parser_comando.h>
#include <server_multithread/server_multithread.h>

//ESTRUCTURAS
typedef struct{
	char *ip;
	char *puerto;
	char *ip_fileSystem;
	char *puerto_fileSystem;
	char *ip_seeds;
	char *puerto_seeds;
	char *tamanio_memoria;
	char *numero_memoria;
}Config_final_data;

//Estructuras de memoria

typedef struct{
	uint16_t key;
	double timestamp;
	char* value;
}marco;

typedef struct{
	t_list* paginas;
}tabla_de_paginas;

typedef struct{
	int numeroPagina;
	bool flagModificado;
	marco* marco;
}pagina;

typedef struct{
	t_list* listaSegmentos;
}tabla_de_segmentos;

typedef struct{
	char* pathTabla;
	t_list* paginas;
}segmento;


//FUNCIONES
t_log* iniciar_logger(bool);
t_config* leer_config(void);
void extraer_data_config(Config_final_data *, t_config* );
void ver_config(Config_final_data *config, t_log* logger_visible);
int conectarLFS(Config_final_data *config, t_log* logger_invisible);
int handshakeLFS(int socketLFS);
int threadConnection(int serverSocket, void *funcionThread);

void inicializarMemoPrincipal(Config_final_data config);



#endif /* MEMORIA_H_ */
