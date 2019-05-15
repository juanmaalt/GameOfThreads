/*
 * APIMemoria.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef APIMEMORIA_H_
#define APIMEMORIA_H_

#include "Memoria.h"

//API	TODO: valores de retorno Resultado
void selectAPI(char*, Comando);
void insertAPI(char*,Comando);
void create(char*,Comando);
void describeAPI();
void dropAPI();
void journalAPI();

#endif /* APIMEMORIA_H_ */
