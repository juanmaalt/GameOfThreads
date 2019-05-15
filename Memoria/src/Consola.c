/*
 * Consola.c
 *
 *  Created on: 14 may. 2019
 *      Author: fdalmaup
 */

#include "Consola.h"

void *recibir_comandos(void *null){
	//pthread_detach(pthread_self());

	Comando *parsed = malloc(sizeof(Comando));
	for(;;){
		char *userInput = readline("> ");
		*parsed = parsear_comando(userInput);

	    if(parsed->valido){
	        switch(parsed->keyword){
	            case SELECT:
	            case INSERT:
	            case CREATE:
	            case DESCRIBE:
	            case DROP:
	            case JOURNAL:
	            	printf("Entro un comando\n");
	            	break;
	            default:
	                fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n", parsed->keyword);
	        }

	        destruir_operacion(*parsed);
	    }else{
	        fprintf(stderr, RED"La linea no es valida"STD"\n");
	    }
	    free(userInput);
	}
	return NULL;
}

