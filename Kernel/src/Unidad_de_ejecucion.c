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





static int exec_string_comando(PCB *pcb){
	Operacion request;
	request.TipoDeMensaje = COMANDO;
	request.Argumentos.COMANDO.comandoParseable = (char*)pcb->data;
	send_msg(socketMemoriaPrincipal, request);
	request = recv_msg(socketMemoriaPrincipal);
	//printf("CPU: %d | Operacion unitaria: %s\n", process_get_thread_id(), userInput);
	loggear_operacion(request);
	//TODO: ver tema free
	return FINALIZO;
}





static int exec_file_lql(PCB *pcb){
	Operacion request;
	request.TipoDeMensaje = COMANDO;
	char buffer[MAX_BUFFER_SIZE_FOR_LQL_LINE];
	char *line = NULL;
	FILE *lql = (FILE *)pcb->data; //Como el FILE nunca se cerro, cada vez que entre, va a continuar donde se habia quedado
	int quantumBuffer = vconfig.quantum(); //Para hacer la llamada una sola vez por cada exec. No se actualiza el quantum en tiempo real, pero se actualiza cuando entra un nuevo script por que ya tiene el valor actualizado

	for(int i=1; i<=quantumBuffer; ++i){
		line = fgets(buffer, MAX_BUFFER_SIZE_FOR_LQL_LINE, lql);
		if(line == NULL || feof(lql)){
			fclose(lql);
			printf("\n");
			//free(pcb->data);
			//free(pcb);
			fclose(lql);
			return FINALIZO;
		}
		request.Argumentos.COMANDO.comandoParseable = line;
		send_msg(socketMemoriaPrincipal, request);
		//printf("CPU: %d | FILE: %p | linea de LQL: %s", process_get_thread_id(), lql, line);
		request = recv_msg(socketMemoriaPrincipal);
		loggear_operacion(request);
	}
	printf("\n");
	sem_wait(&meterEnReadyDeAUno);
	desalojar(pcb);
	sem_post(&meterEnReadyDeAUno);
	sem_post(&scriptEnReady); //Ya que se metio a la lista de vuelta
	if(line != NULL)
		;//free(line);
	simular_retardo();
	return DESALOJO;
}

static void loggear_operacion(Operacion op){
	switch(op.TipoDeMensaje){
	case TEXTO_PLANO:
		log_info(logger_visible,"CPU: %d | %s", process_get_thread_id(), op.Argumentos.TEXTO_PLANO.texto);
		return;
	case COMANDO:
		log_info(logger_visible,"CPU: %d | %s", process_get_thread_id(), op.Argumentos.COMANDO.comandoParseable);
		return;
	case REGISTRO:
		log_info(logger_visible,"CPU: %d | Timestamp: %llu, Key: %d, Value: %s", process_get_thread_id(), op.Argumentos.REGISTRO.timestamp, op.Argumentos.REGISTRO.key, op.Argumentos.REGISTRO.value);
		return;
	case ERROR:
		log_error(logger_error,"CPU: %d | Kernel panic: ", process_get_thread_id(), op.Argumentos.ERROR.mensajeError);
		return;
	}
}

