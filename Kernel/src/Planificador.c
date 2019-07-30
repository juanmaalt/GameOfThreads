/*
 * Planificador.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Planificador.h"

//FUNCIONES: Privadas
static int iniciar_unidades_de_ejecucion();
static int iniciar_describe_automatico();

int iniciar_planificador(){
	if(iniciar_unidades_de_ejecucion() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudieron iniciar las unidades de ejecucion");

	if(iniciar_sistema_de_criterios() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudo iniciar el sistema de criterios");

	if(iniciar_gossiping() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_gossiping: no se pudo iniciar el sistema gossiping del kernel");

	if(iniciar_describe_automatico() == EXIT_FAILURE){
		RETURN_ERROR("Planificador.c: iniciar_describe_automatico: no se pudo iniciar el describe automatico")
	}

	colaDeReady = queue_create();
	sem_post(&disponibilidadPlanificador); //No queremos que la consola agregue algo a la cola de news si todavia no existe la cola de ready

	sem_wait(&dormirProcesoPadre);
	return EXIT_SUCCESS;
}





int new(PCB_DataType tipo, void *data, char *nombreArchivoLQL){
	PCB *pcb = malloc(sizeof(PCB));
	pcb->data = data;
	pcb->simbolicIP = 0;
	switch(tipo){
	case STRING_COMANDO:
		pcb->tipo = STRING_COMANDO;
		pcb->nombreArchivoLQL = string_from_format(nombreArchivoLQL);
		break;
	case FILE_LQL:
		pcb->tipo = FILE_LQL;
		pcb->nombreArchivoLQL = string_from_format(nombreArchivoLQL);
		break;
	default:
		RETURN_ERROR("Planificador.c: new: no se reconoce el tipo de dato a ejecutar");
	}
	queue_push(colaDeReady, pcb);
	sem_post(&scriptEnReady); //Como esta funcion va a ser llamada por la consola, el semaforo tambien tiene que ser compartido por el proceso consola
	return EXIT_SUCCESS;
}





static int iniciar_unidades_de_ejecucion(){
	idsExecInstances = list_create();
	for(int i=0; i<fconfig.multiprocesamiento; ++i){
		pthread_t *id = malloc(sizeof(pthread_t));
		int res = pthread_create(id, NULL, exec, NULL);
		if(res != 0){
			RETURN_ERROR("Planificador.c: iniciar_planificacion: fallo la creacion de un proceso.");
		}
		list_add(idsExecInstances, id);
	}
	return EXIT_SUCCESS;
}





static int iniciar_describe_automatico(){
	if((describeAutomatico = pthread_create(&describeAutomatico, NULL, describe_automatico, NULL)) != 0)
		RETURN_ERROR("Planificador.c: iniciar_describe_automatico: Error al crear hilo de describe automatico");
	return EXIT_SUCCESS;
}





void *describe_automatico(void *null){
	for(;;){
		usleep(vconfig.refreshMetadata*1000);
		Memoria *memoria = elegir_cualquiera();
		if(memoria == NULL) continue;
		int socketMemoria = solicitar_socket(memoria);
		Operacion operacion;
		operacion.TipoDeMensaje = COMANDO;
		operacion.Argumentos.COMANDO.comandoParseable = string_from_format("DESCRIBE");
		send_msg(socketMemoria, operacion);
		destruir_operacion(operacion);
		operacion = recv_msg(socketMemoria);
		destruir_operacion(operacion); //No hago nada
	}
	return NULL;
}





void ejecutar_journal(void *memoria){
	Memoria *mem = (Memoria*)memoria;
	Operacion op;
	op.TipoDeMensaje = COMANDO;
	op.Argumentos.COMANDO.comandoParseable = string_from_format("JOURNAL");
	int socket = solicitar_socket(mem);
	send_msg(socket, op);
	destruir_operacion(op);
}





PCB *seleccionar_siguiente(){
	return (PCB*)queue_pop(colaDeReady);
}

void desalojar(PCB *pcb){
	queue_push(colaDeReady, pcb);
}

void simular_retardo(void){
	usleep(vconfig.retardo * 1000);
}
