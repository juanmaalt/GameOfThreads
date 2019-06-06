/*
 * Unidad_de_ejecucion.h
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#ifndef UNIDAD_DE_EJECUCION_H_
#define UNIDAD_DE_EJECUCION_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/process.h>
#include "Planificador.h"

typedef struct pcb_t PCB;
typedef enum flag_e Flag;

typedef enum{
	DESALOJO,
	FINALIZO
}ResultadoEjecucionInterno;

//FUNCIONES: Publicas
void *exec(void *null);

//FUNCIONES: Privadas
static int exec_file_lql(PCB *pcb);
static int exec_string_comando(PCB *pcb);

#endif /* UNIDAD_DE_EJECUCION_H_ */
