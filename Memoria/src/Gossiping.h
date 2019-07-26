/*
 * Gossiping.h
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */

#ifndef GOSSIPING_H_
#define GOSSIPING_H_


#include "Memoria.h"

#include <parser/compresor_direccion.h>

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

pthread_mutex_t mutexGossiping;

char **IPs;
char **IPsPorts;

int iniciar_gossiping();

void liberarIPs(char** );
void quitarCaracteresPpioFin(char* );
void conectarConSeed();
void *conectar_seeds(void*);

void ConsultoPorMemoriasConocidas(int );
Operacion recibir_gossiping (Operacion);
static knownMemory_t *machearMemoria(int );
void chequeo_memorias_en_lista_activas (void);
#endif /* GOSSIPING_H_ */
