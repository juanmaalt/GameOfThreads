/*
 * Consola.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Consola.h"

void *recibir_comandos(void *null){
	pthread_detach(pthread_self());
	sem_wait(&disponibilidadPlanificador);
	Comando *parsed = malloc(sizeof(Comando));
	for(;;){
		char *userImput = readline("> ");
		*parsed = parsear_comando(userImput);

	    if(parsed->valido){
	        switch(parsed->keyword){
	            case SELECT:
	            case INSERT:
	            case CREATE:
	            case DESCRIBE:
	            case DROP:
	            case JOURNAL:
	            case ADDMEMORY:
	            	new(COMANDO, parsed);
	            	break;
	            case RUN:
	            	if(cargar_lql(parsed->argumentos.RUN.path) == EXIT_FAILURE){
	            		printf(RED"Consola.c: recibir_comandos: hubo un problema en el archivo LQL"STD"\n");
	            	}
	                break;
	            case METRICS:
	            	//TODO: comando metrics
	                break;
	            default:
	                fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n", parsed->keyword);
	        }

	        destruir_operacion(*parsed);
	    }else{
	        fprintf(stderr, RED"La linea no es valida"STD"\n");
	    }
	    free(userImput);
	}
	return NULL;
}

int cargar_lql(char *path){
	FILE *lql = fopen(path, "r");
	if(lql == NULL){
		printf(RED"Consola.c: cargar_lql: el path indicado no es valido"STD"\n");
		return EXIT_FAILURE;
	}
	char buffer[100];
	while(!feof(lql)){
		if(comando_validar(parsear_comando(fgets(buffer, 100, lql))) == EXIT_FAILURE){ //TODO: posible malgasto de memoria, no perder de vista
			printf(RED"Consola.c: cargar_lql: el archivo LQL tiene una o mas lineas invalidas"STD"\n");
			return EXIT_FAILURE;
		}
	}
	new(LQL, lql);
	//TODO: acordarse de cerrar el archivo una vez que se ejecute
	return EXIT_SUCCESS;
}
