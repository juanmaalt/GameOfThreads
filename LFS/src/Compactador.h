/*
 * Compactador.h
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#ifndef COMPACTADOR_H_
#define COMPACTADOR_H_

#include "adhoc/FuncionesComp.h"
//#include "adhoc/FuncionesAPI.h"
#include <commons/config.h>


typedef struct dirent EntradaDirectorio;

/*GLOBALES*/

/*FUNCIONES*/
void* compactar(void* nombreTabla);

void bloquearTabla(char *tabla);
void desbloquearTabla(char *tabla);

void bloquearSelect(char *tabla);
void desbloquearSelect(char *tabla);

void tryExecute(char *tabla);
void tryExecuteSelect(char *tabla);

void reiniciarSemaforos(char *tabla);

void tomarTiempoInicio(char *tabla);
void tomarTiempoFin(char *tabla);

void loggearTiempoCompactacion(char *tabla);


#endif /* COMPACTADOR_H_ */
