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
#include <random/random_numbers.h>
#include "Planificador.h"

typedef struct pcb_t PCB;
typedef enum flag_e Flag;

typedef enum{
	DESALOJO,
	FINALIZO,
	INSTRUCCION_ERROR,
	CONTINUAR
}ResultadoEjecucionInterno;

//FUNCIONES: Publicas
void *exec(void *null);

#endif /* UNIDAD_DE_EJECUCION_H_ */
