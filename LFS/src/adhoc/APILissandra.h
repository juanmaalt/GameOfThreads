/*
 * APILissandra.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef APILISSANDRA_H_
#define APILISSANDRA_H_

#include "FuncionesAPI.h"

//API
Operacion selectAPI(Comando comando);
Operacion insertAPI(Comando comando);
Operacion createAPI(Comando);
Operacion describeAPI(Comando);
Operacion dropAPI(Comando);

#endif /* APILISSANDRA_H_ */
