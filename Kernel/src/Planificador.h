/*
 * Planificador.h
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#define MAX_BUFFER_SIZE_FOR_LQL_LINE 100

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <commons/collections/queue.h>
#include "Kernel.h"
#include "Unidad_de_ejecucion.h"

//ESTRUCTURAS
typedef enum PCB_DataType_e{
	STRING_COMANDO,
	FILE_LQL
}PCB_DataType;


typedef struct pcb_t{
	void *data;
	PCB_DataType tipo;
}PCB;



//GLOBALES
t_queue *colaDeReady;


//FUNCIONES: Globales
int iniciar_planificador();
int new(PCB_DataType tipo, void *data); //Esta funcion va a ser ejecutada desde la consola por el proceso de consola
PCB *seleccionar_siguiente();
void desalojar(PCB *pcb);
void simular_retardo(void);

//FUNCIONES: Privadas
int comunicarse_con_memoria();
static int iniciar_unidades_de_ejecucion();


#endif /* PLANIFICADOR_H_ */
