/*
 * Unidad_de_ejecucion.c
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#include "Unidad_de_ejecucion.h"

void *exec(void *null){
	pthread_detach(pthread_self());
	int resultado;
	for(;;){
		//La cpu por default esta disponible si esta en este wait
		sem_wait(&scriptEnReady);
		sem_wait(&extraerDeReadyDeAUno);
		PCB *pcb = seleccionar_siguiente();
		sem_post(&extraerDeReadyDeAUno);
		switch(pcb->tipo){
		case STRING_COMANDO:
			resultado = exec_string_comando(pcb);
			break;
		case FILE_LQL:
			resultado = exec_file_lql(pcb);
			break;
		}
	}
	return NULL;
}

int exec_string_comando(PCB *pcb){
	char *userInput = (char*)pcb->data;
	Comando comando = parsear_comando(userInput);
	printf("CPU: %d | Operacion unitaria: %s\n", process_get_thread_id(), userInput);
	destruir_operacion(comando);
	if(userInput != NULL)
		free(userInput);
	simular_retardo();
	return EJECUTO;
}

int exec_file_lql(PCB *pcb){
	Comando comando;
	char buffer[MAX_BUFFER_SIZE_FOR_LQL_LINE];
	char *line;
	FILE *lql;

	lql = (FILE *)pcb->data; //Como el FILE nunca se cerro, cada vez que entre, va a continuar donde se habia quedado
	for(int i=1; i<=vconfig.quantum; ++i){
		line = fgets(buffer, MAX_BUFFER_SIZE_FOR_LQL_LINE, lql);
		if(line == NULL || feof(lql)){
			fclose(lql);
			printf("\n");
			//free(pcb->data);
			//free(pcb);
			return FINALIZO;
		}
		comando = parsear_comando(line);
		printf("CPU: %d | FILE: %p | linea de LQL: %s", process_get_thread_id(), lql, line);
		destruir_operacion(comando);
	}printf("\n");
	sem_wait(&meterEnReadyDeAUno);
	desalojar(pcb);
	sem_post(&meterEnReadyDeAUno);
	sem_post(&scriptEnReady); //Ya que se metio a la lista de vuelta
	if(line != NULL)
		;//free(line);
	simular_retardo();
	return DESALOJO;
}

