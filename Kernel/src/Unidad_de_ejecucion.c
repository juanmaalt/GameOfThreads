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
		sem_wait(&entrarAReadyDeAUno);
		PCB *pcb = seleccionar_siguiente();
		sem_post(&entrarAReadyDeAUno);

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

	return EXIT_SUCCESS;
}

int exec_file_lql(PCB *pcb){
	Comando *comando;
	for(int i=1; i<=config.quantum; ++i){
		comando = decodificar_siguiente_instruccion(pcb->data);
		//codigo
		destruir_operacion(comando);
	}
	if(comando != NULL)
		free(comando);
	return EXIT_SUCCESS;
}
