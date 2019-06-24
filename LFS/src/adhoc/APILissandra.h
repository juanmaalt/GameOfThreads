/*
 * APILissandra.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef APILISSANDRA_H_
#define APILISSANDRA_H_

#include "FuncionesAPI.h"

/*FUNCIONES API*/
Operacion selectAPI(Comando comando);
Operacion insertAPI(Comando comando);
Operacion createAPI(Comando comando);
Operacion describeAPI(Comando comando);
Operacion dropAPI(Comando comando);

#endif /* APILISSANDRA_H_ */
