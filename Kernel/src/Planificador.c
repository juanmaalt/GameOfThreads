/*
 * Planificador.c
 *
 *  Created on: 7 may. 2019
 *      Author: facundosalerno
 */

#include "Planificador.h"

int iniciar_planificador(){
	if(iniciar_unidades_de_ejecucion() == EXIT_FAILURE)
		RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudieron iniciar las unidades de ejecucion");
	//if(comunicarse_con_memoria() == EXIT_FAILURE)
		//RETURN_ERROR("Planificador.c: iniciar_planificador: no se pudo establecer una conexion con la memoria principal");

	colaDeReady = queue_create();
	sem_post(&disponibilidadPlanificador); //No queremos que la consola agregue algo a la cola de news si todavia no existe la cola de news

	sem_wait(&dormirProcesoPadre);
	RETURN_ERROR("Planificador.c: iniciar_planificador: (Warning) el proceso padre se desperto y podria desencadenar la finalizacion del kernel");
	return EXIT_SUCCESS;
}






int new(PCB_DataType tipo, void *data){
	PCB *pcb = malloc(sizeof(PCB));
	pcb->data = data; //pcb->data almacena la direccion data
	switch(tipo){
	case STRING_COMANDO:
		pcb->tipo = STRING_COMANDO;
		break;
	case FILE_LQL:
		pcb->tipo = FILE_LQL;
		break;
	default:
		RETURN_ERROR("Planificador.c: new: no se reconoce el tipo de dato a ejecutar");
	}
	queue_push(colaDeReady, pcb);
	sem_post(&scriptEnReady); //Como esta funcion va a ser llamada por la consola, el semaforo tambien tiene que ser compartido por el proceso consola
	return EXIT_SUCCESS;
}





static int comunicarse_con_memoria(){
	for(int i=1; i<=6; ++i){
		if((socketMemoriaPrincipal = connect_to_server(fconfig.ip_memoria, fconfig.puerto_memoria)) == EXIT_FAILURE){
			log_error(logger_error, "Planificador.c: comunicarse_con_memoria: error al conectarse al servidor memoria... Reintentando (%d/6)", i);
			sleep(1);
		}else{
			log_info(logger_visible, "Conectado a la memoria principal");
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}





static int iniciar_unidades_de_ejecucion(){
	idsExecInstances = list_create();
	for(int i=0; i<fconfig.multiprocesamiento; ++i){
		pthread_t *id = malloc(sizeof(pthread_t)); //Lo hago asi por que los salames que hicieron la funcion list_add nada mas linkean el puntero, no le copian el valor. Por ende voy a necesitar un malloc de int por cada valor que quiera guardar, y no hacerles free de nada
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
