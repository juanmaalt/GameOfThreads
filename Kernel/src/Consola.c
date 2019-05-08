/*
 * Consola.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Consola.h"

void recibir_comandos(){
	while(1){
		char *userImput = readline("Soy la consola> ");
		free(userImput);
	}
	exit(EXIT_SUCCESS);
}
