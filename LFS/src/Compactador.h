/*
 * Compactador.h
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#ifndef COMPACTADOR_H_
#define COMPACTADOR_H_

#include "adhoc/FuncionesComp.h"
#include <commons/config.h>

/*GLOBALES*/

/*FUNCIONES*/
void* compactar(void* nombreTabla);
void wait_semaforo_tabla(char *tabla);
void post_semaforo_tabla(char *tabla);


#endif /* COMPACTADOR_H_ */
