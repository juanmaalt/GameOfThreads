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
	char *ipandport ;
	//char *ip_port;
}knownMemory_t;

t_list* listaMemoriasConocidas;


pthread_t idGossipSend;
pthread_t idGossipReciv;


char **IPs;
char **IPsPorts;

int iniciar_gossiping();

void liberarIPs(char** );
void quitarCaracteresPpioFin(char* );
void conectarConSeed();
void *conectar_seeds(void*);
void *recibir_seeds(void*);
void ConsultoPorMemoriasConocidas(int );
Operacion recibir_gossiping (Operacion);
#endif /* GOSSIPING_H_ */
