/*
 * FuncionesComp.h
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#ifndef ADHOC_FUNCIONESCOMP_H_
#define ADHOC_FUNCIONESCOMP_H_

#include "../Lissandra.h"
#include "APILissandra.h"

void cambiarNombreFilesTemp(char* pathTabla);
void agregarTablaEnDiccCompactacion(char* nombreTabla);
void sacarTablaDeDiccCompactacion(char* nombreTabla);

#endif /* ADHOC_FUNCIONESCOMP_H_ */
