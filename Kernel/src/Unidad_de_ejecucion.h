/*
 * Unidad_de_ejecucion.h
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#ifndef UNIDAD_DE_EJECUCION_H_
#define UNIDAD_DE_EJECUCION_H_
#define RED "\x1b[31m"
#define STD "\x1b[0m"

#include <stdio.h>
#include <stdlib.h>
#include <commons/process.h>
#include "Planificador.h"

typedef struct pcb_t PCB;
typedef enum flag_e Flag;

typedef enum{
	EJECUTO,
	DESALOJO,
	FINALIZO
}ResultadoEjecucionInterno;

//FUNCIONES
void *exec(void *null);
int exec_file_lql(PCB *pcb);
int exec_string_comando(PCB *pcb);

#endif /* UNIDAD_DE_EJECUCION_H_ */
