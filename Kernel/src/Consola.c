/*
 * Consola.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Consola.h"
#include <parser/compresor_direccion.h>

//FUNCIONES: Privadas
static int new_lql(char* path);
static int run_all_lql(char *dirPath);
static int new_comando(PCB_DataType tipo, char *data);

void *recibir_comandos(void *null){
	pthread_detach(pthread_self());
	sem_wait(&disponibilidadPlanificador);
	for(;;){
		Comando *parsed = malloc(sizeof(Comando));
		char *userImput = readline("");
		*parsed = parsear_comando(userImput);
	    if(parsed->valido){
	        switch(parsed->keyword){
	            case SELECT:
	            case INSERT:
	            case CREATE:
	            case DESCRIBE:
	            case DROP:
	            case JOURNAL:
	            	if(new_comando(STRING_COMANDO, userImput) == EXIT_FAILURE){
	            		log_error(logger_error, "Consola.c: recibir_comandos: el comando no pudo ingresar a new");
	            		log_error(logger_invisible, "Consola.c: recibir_comandos: el comando no pudo ingresar a new");
	            	}//Aca no se libera userImput, se libera en unidad_de_ejecucion.c con el nombre de pcb->data
	            	break;
	            case ADDMEMORY:
	            	if(add_memory(parsed->argumentos.ADDMEMORY.numero, parsed->argumentos.ADDMEMORY.criterio) == EXIT_FAILURE){
	            		log_error(logger_error, "Consola.c: add_memory: fallo la asociacion de la memoria al criterio");
	            		log_error(logger_invisible, "Consola.c: add_memory: fallo la asociacion de la memoria al criterio");
	            	}
	            	free(userImput);
	            	break;

	            case RUN:
	            	if(new_lql(parsed->argumentos.RUN.path) == EXIT_FAILURE){
	            		log_error(logger_error, "Consola.c: recibir_comandos: hubo un problema en el archivo LQL");
	            		log_error(logger_invisible, "Consola.c: recibir_comandos: hubo un problema en el archivo LQL");
	            	}
	            	free(userImput);
	                break;

	            case RUN_ALL:
	            	if(run_all_lql(parsed->argumentos.RUN_ALL.dirPath) == EXIT_FAILURE){
	            		log_error(logger_error, "Consola.c: recibir_comandos: hubo un problema en el directorio que contiene los archivos LQL");
	            		log_error(logger_invisible, "Consola.c: recibir_comandos: hubo un problema en el directorio que contiene los archivos LQL");
	            	}
	            	free(userImput);
	            	break;

	            case METRICS:
	            	ver_metricas();
	            	free(userImput);
	                break;

	            case METRICS_STOP:
	            	no_ver_metricas();
	            	free(userImput);
	            	break;

	            default:
            		log_error(logger_error, "Consola.c: recibir_comandos: no se pude interpretar el enum");
            		log_error(logger_invisible, "Consola.c: recibir_comandos: no se pude interpretar el enum");
            		free(userImput);
	        }
	        destruir_comando(*parsed);
	    }else{
    		log_error(logger_error, "Consola.c: recibir_comandos: la linea no es valida");
    		log_error(logger_invisible, "Consola.c: recibir_comandos: la linea no es valida");
	    }
	    free(parsed);
	}
	return NULL;
}





static int new_comando(PCB_DataType tipo, char* data){
	return new(tipo, (void*)data, "Request unitaria");
}





static int new_lql(char *path){
	FILE *lql = fopen(path, "r");
	if(lql == NULL){
		RETURN_ERROR("Consola.c: new_lql: el path indicado no es valido");
	}

	char buffer[MAX_BUFFER_SIZE_FOR_LQL_LINE];
	char *line;
	Comando comando;//Se usa para verificar que el lql contenga comandos validos
	for(int i=1; !feof(lql) && (line = fgets(buffer, MAX_BUFFER_SIZE_FOR_LQL_LINE, lql)) != NULL; ++i){
		comando = parsear_comando(line);
		if(comando_validar(comando) == EXIT_FAILURE){ //TODO: posible malgasto de memoria, no perder de vista
			destruir_comando(comando);
			char *temp = remover_new_line(line);
    		log_error(logger_error, "Consola.c: new_lql: la linea %d: '%s' del LQL '%s' es invalida", i, temp, path);
    		log_error(logger_invisible, "Consola.c: new_lql: la linea %d: '%s' del LQL '%s; es invalida", i, temp, path);
    		free(temp);
			return EXIT_FAILURE;
		}
		destruir_comando(comando);
	}

	fseek(lql, 0, SEEK_SET);//Restauro el puntero del archivo al inicio
	if(new(FILE_LQL, (void*)lql, path) == EXIT_FAILURE){
		RETURN_ERROR("Consola.c: new_lql: el archivo LQL no pudo ingresar a new");
	}
	//el archivo FILE lql se cierra en unidad_de_ejecucion.c, exec_file_lql()
	return EXIT_SUCCESS;
}





static int run_all_lql(char *dirPath){
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (dirPath)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
		  char *archivo = string_from_format("%s/%s", dirPath, ent->d_name);
		  new_lql(archivo);
		  free(archivo);
		}
	 	closedir (dir);
	} else {
		/* could not open directory */
		printf(RED"No se encontro el directorio"STD"\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
