/*
 * Consola.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include "../Lissandra.h"

/*FUNCIONES*/
void *recibir_comandos(void *);
void mostrarRetorno(Operacion retorno);


#endif /* CONSOLA_H_ */
