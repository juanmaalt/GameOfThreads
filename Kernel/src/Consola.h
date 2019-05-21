/*
 * Consola.h
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_
#define MAX_BUFFER_SIZE_FOR_LQL_LINE 100

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "Kernel.h"

typedef enum PCB_DataType_e PCB_DataType;

//FUNCIONES: Globales
void *recibir_comandos(void *);

//FUNCIONES: Privadas
static int new_lql(char* path);
static int new_comando(PCB_DataType tipo, char *data);

//FUNCIONES: Testeo
void funcion_loca_de_testeo_de_concurrencia(void);

#endif /* CONSOLA_H_ */
