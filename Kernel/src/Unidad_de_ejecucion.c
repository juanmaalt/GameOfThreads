/*
 * Unidad_de_ejecucion.c
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#include "Unidad_de_ejecucion.h"

void *exec(void *null){
	pthread_detach(pthread_self());
	for(;;){
		//La cpu por default esta disponible
		sem_wait(&ordenDeEjecucion);
		//Codigo
		sem_post(&unidadDeEjecucionDisponible);
	}
	return NULL;
}
