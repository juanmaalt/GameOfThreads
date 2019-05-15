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
		char *userImput = readline("> "); //TODO: el userInput va a pasar a formar parte del PCB probablemente, asi que tenerlo en cuenta para la liberacion, todavia no se como lo voy a hacer
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
	            	if(new_comando(STRING_COMANDO, userImput) == EXIT_FAILURE){
	            		printf(RED"Consola.c: recibir_comandos: el comando no puso ingresar a new"STD"\n");
	            	}
	            	break;
	            case RUN:
	            	if(new_lql(parsed->argumentos.RUN.path) == EXIT_FAILURE){
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
	    //free(userImput);
	}
	return NULL;
}

int new_comando(TipoDeMensaje tipo, char* data){
	return new(tipo, (void*)data);
}

int new_lql(char *path){
	FILE *lql = fopen(path, "r");
	if(lql == NULL){
		printf(RED"Consola.c: new_lql: el path indicado no es valido"STD"\n");
		return EXIT_FAILURE;
	}
	char buffer[MAX_BUFFER_SIZE_FOR_LQL_LINE];
	char *line;
	while(!feof(lql) && (line = fgets(buffer, MAX_BUFFER_SIZE_FOR_LQL_LINE, lql)) != NULL){
		if(comando_validar(parsear_comando(line)) == EXIT_FAILURE){ //TODO: posible malgasto de memoria, no perder de vista
			printf(RED"Consola.c: new_lql: el archivo LQL tiene una o mas lineas invalidas"STD"\n");
			if(line != NULL)
				free(line);
			return EXIT_FAILURE;
		}
		if(line != NULL)
			free(line);
	}
	if(new(FILE_LQL, lql) == EXIT_FAILURE){
		printf(RED"Consola.c: new_lql: el archivo LQL no pudo ingresar a new"STD"\n");
		return EXIT_FAILURE;
	}
	//TODO: acordarse de cerrar el archivo una vez que se ejecute
	return EXIT_SUCCESS;
}
