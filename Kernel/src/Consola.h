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

//FUNCIONES
void *recibir_comandos(void *);
int new_lql(char* path);
int new_comando(TipoDeMensaje tipo, char *data);

#endif /* CONSOLA_H_ */
