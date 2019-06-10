/*
 * Consola.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Consola.h"

//FUNCIONES: Privadas
static int new_lql(char* path);
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
	            	break;

	            case RUN:
	            	if(string_equals_ignore_case(parsed->argumentos.RUN.path, "TEST")){
	            		funcion_loca_de_testeo_de_concurrencia();
	            		break;
	            	}
	            	if(new_lql(parsed->argumentos.RUN.path) == EXIT_FAILURE){
	            		log_error(logger_error, "Consola.c: recibir_comandos: hubo un problema en el archivo LQL");
	            		log_error(logger_invisible, "Consola.c: recibir_comandos: hubo un problema en el archivo LQL");
	            	}
	            	free(userImput);
	                break;

	            case METRICS:
	            	if(parsed->argumentos.METRICS.stop == NULL)
	            		ver_metricas();
	            	else
	            		no_ver_metricas();
	                break;

	            default:
            		log_error(logger_error, "Consola.c: recibir_comandos: no se pude interpretar el enum");
            		log_error(logger_invisible, "Consola.c: recibir_comandos: no se pude interpretar el enum");
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
	return new(tipo, (void*)data);
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
    		log_error(logger_error, "Consola.c: new_lql: la linea %d del LQL es invalida", i);
    		log_error(logger_invisible, "Consola.c: new_lql: la linea %d del LQL es invalida", i);
			return EXIT_FAILURE;
		}
		destruir_comando(comando);
	}

	fseek(lql, 0, SEEK_SET);//Restauro el puntero del archivo al inicio
	if(new(FILE_LQL, (void*)lql) == EXIT_FAILURE){
		RETURN_ERROR("Consola.c: new_lql: el archivo LQL no pudo ingresar a new");
	}
	//el archivo FILE lql se cierra en unidad_de_ejecucion.c, exec_file_lql()
	return EXIT_SUCCESS;
}





void funcion_loca_de_testeo_de_concurrencia(void){
	/*Hacer en la carpeta donde se encuentre el ejecutable 4 archivos que se llamen:
	 * lql1.lql
	 * lql2.lql
	 * lql3.lql
	 * lql4.lql
	 * Inovcar la funcion em algun comando para probarla
	 * */
	new_lql("lql/lql1.lql");
	new_lql("lql/lql2.lql");
	new_lql("lql/lql3.lql");
	new_lql("lql/lql4.lql");
}
