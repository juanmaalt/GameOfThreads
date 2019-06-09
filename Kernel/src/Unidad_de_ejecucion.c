/*
 * Unidad_de_ejecucion.c
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#include "Unidad_de_ejecucion.h"

//FUNCIONES: Privadas. No van en el header.
static int exec_file_lql(PCB *pcb);
static int exec_string_comando(PCB *pcb);
static void loggear_operacion(Operacion op);
static int conectarse_con_memoria_segun_request(PCB *pcb);
static int comunicarse_con_memoria();

void *exec(void *null){
	pthread_detach(pthread_self());
	for(;;){
		//La cpu por default esta disponible si esta en este wait
		sem_wait(&scriptEnReady);
		sem_wait(&extraerDeReadyDeAUno);
		PCB *pcb = seleccionar_siguiente();
		socketTarget = conectarse_con_memoria_segun_request(pcb);
		sem_post(&extraerDeReadyDeAUno);
		switch(pcb->tipo){
		case STRING_COMANDO:
			exec_string_comando(pcb);
			break;
		case FILE_LQL:
			exec_file_lql(pcb);
			break;
		}
		close(socketTarget);
	}
	return NULL;
}





static int conectarse_con_memoria_segun_request(PCB *pcb){
	//hacer algo loco de para elegir segun criterio. Le paso la pcb por que tiene la request
	return comunicarse_con_memoria(fconfig.ip_memoria, fconfig.puerto_memoria);
}





static int comunicarse_con_memoria(char *ip, char *puerto){
	int socketServer;
	if((socketServer = connect_to_server(ip, puerto)) == EXIT_FAILURE){
		log_error(logger_error, "Planificador.c: comunicarse_con_memoria: error al conectarse al servidor memoria %s:%s", ip, puerto);
		return EXIT_FAILURE;
	}
	log_info(logger_visible, "Conectado a la memoria %s:%s", ip, puerto);
	return socketServer;
}





static int exec_string_comando(PCB *pcb){
	Operacion request;
	request.TipoDeMensaje = COMANDO;
	request.Argumentos.COMANDO.comandoParseable = (char*)pcb->data;
	send_msg(socketTarget, request);
	request = recv_msg(socketTarget);
	loggear_operacion(request);
	destruir_operacion(request);
	free(pcb->data);
	free(pcb);
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
			printf("\n");
			fclose(lql);
			free(pcb);
			return FINALIZO;
		}
		request.Argumentos.COMANDO.comandoParseable = line;
		send_msg(socketTarget, request);
		request = recv_msg(socketTarget);
		loggear_operacion(request);
		destruir_operacion(request);
	}
	printf("\n");
	sem_wait(&meterEnReadyDeAUno);
	desalojar(pcb);
	sem_post(&meterEnReadyDeAUno);
	sem_post(&scriptEnReady); //Ya que se metio a la lista de vuelta
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
		log_error(logger_error,"CPU: %d | Kernel panic: %s", process_get_thread_id(), op.Argumentos.ERROR.mensajeError);
		return;
	}
}

