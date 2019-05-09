/*
 * Planificador.h
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_
#define RED "\x1b[31m"
#define STD "\x1b[0m"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <commons/collections/queue.h>
#include "Kernel.h"
#include "Unidad_de_ejecucion.h"

//ESTRUCTURAS
typedef struct{
	void *data;
	TipoDeMensaje tipo;
	unsigned short quantumRemanente;
}PCB;

//GLOBALES
t_queue *colaDeNew;

//FUNCIONES
int iniciar_planificador();
int iniciar_unidades_de_ejecucion();
int new(TipoDeMensaje tipo, void *data); //Esta funcion va a ser ejecutada desde la consola por el proceso de consola
int ready(); //Esta es la funcion donde desencadena el proceso padre


#endif /* PLANIFICADOR_H_ */
