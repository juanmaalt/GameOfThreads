/*
 * Consola.h
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "Kernel.h"

typedef enum PCB_DataType_e PCB_DataType;

//FUNCIONES: Globales
void *recibir_comandos(void *);

//FUNCIONES: Testeo
void funcion_loca_de_testeo_de_concurrencia(void);

#endif /* CONSOLA_H_ */
