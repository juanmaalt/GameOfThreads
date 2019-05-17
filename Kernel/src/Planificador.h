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
typedef enum{
	STRING_COMANDO,
	FILE_LQL
}PCB_DataType;

typedef struct{
	void *data;
	PCB_DataType tipo;
	unsigned short quantumRemanente;
}PCB;


//GLOBALES
t_queue *colaDeReady;


//FUNCIONES
int iniciar_planificador();
int comunicarse_con_memoria();
int iniciar_unidades_de_ejecucion();
int new(PCB_DataType tipo, void *data); //Esta funcion va a ser ejecutada desde la consola por el proceso de consola
PCB *seleccionar_siguiente();


#endif /* PLANIFICADOR_H_ */
