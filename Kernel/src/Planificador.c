/*
 * Planificador.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Planificador.h"

void iniciar_procesador(){
	printf("Hola, soy el proceso %d\n", getpid());
	exit(EXIT_SUCCESS);
}
