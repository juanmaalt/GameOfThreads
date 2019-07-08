/*
 * Planificador.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Planificador.h"

//FUNCIONES: Privadas. No van en el header
static int iniciar_unidades_de_ejecucion();

int iniciar_planificador(){
	if(iniciar_unidades_de_ejecucion() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudieron iniciar las unidades de ejecucion");
	if(verificar_memoria_principal() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudo establecer una conexion con la memoria principal");

	if(iniciar_sistema_de_criterios() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudo iniciar el sistema de criterios");

	colaDeReady = queue_create();
	sem_post(&disponibilidadPlanificador); //No queremos que la consola agregue algo a la cola de news si todavia no existe la cola de news

	if(iniciar_gossiping() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_gossiping: no se pudo iniciar el sistema gossiping del kernel");

	sem_wait(&dormirProcesoPadre);
	return EXIT_SUCCESS;
}





int verificar_memoria_principal(){
	int socket;
	if((socket = connect_to_server(fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal)) == EXIT_FAILURE)
		return EXIT_FAILURE;
	close(socket);
	Memoria *principal = malloc(sizeof(Memoria));
	principal->ip = string_from_format(fconfig.ip_memoria_principal);
	principal->puerto = string_from_format(fconfig.puerto_memoria_principal);
	principal->numero = fconfig.numero_memoria_principal;
	list_add(memoriasExistentes, principal);
}





int new(PCB_DataType tipo, void *data, char *nombreArchivoLQL){
	PCB *pcb = malloc(sizeof(PCB));
	pcb->data = data; //pcb->data almacena la direccion data
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
		pthread_t *id = malloc(sizeof(pthread_t)); //la funcion list_add nada mas linkean el puntero, no le copian el valor. Por ende voy a necesitar un malloc de int por cada valor que quiera guardar, y no hacerles free de nada
		//TODO: todos estos mallocs de int se liberan supuestamente al finalizar el programa, pero no perderlo de vista xq podria haber algun error purulando
		int res = pthread_create(id, NULL, exec, NULL);
		if(res != 0){
			RETURN_ERROR("Kernel.c: iniciar_planificacion: fallo la creacion de un proceso");
		}
		list_add(idsExecInstances, id);
	}
	return EXIT_SUCCESS;
}





PCB *seleccionar_siguiente(){
	return (PCB*)queue_pop(colaDeReady);
}

void desalojar(PCB *pcb){
	queue_push(colaDeReady, pcb);
}

void simular_retardo(void){
	usleep(vconfig.retardo());
}
