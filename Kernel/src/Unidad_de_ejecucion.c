/*
 * Unidad_de_ejecucion.c
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#include "Unidad_de_ejecucion.h"

void *exec(void *null){
	pthread_detach(pthread_self());
	/*TODO: ver como conviene resolver esto, si con semaforos y tener un mismo comportamiento
	 * para cada procesador, o hacer algo mas comlejo
	 */
	return NULL;
}
