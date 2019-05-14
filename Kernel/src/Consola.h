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

//FUNCIONES
void *recibir_comandos(void *);
int cargar_lql(char* path);

#endif /* CONSOLA_H_ */
