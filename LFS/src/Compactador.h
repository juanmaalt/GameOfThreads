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

void compactar(char* nombreTabla);
void leerTemporal(char* pathTemp, int particiones, char* nombreTabla);
char* obtenerListaDeBloques(int particion, char* nombreTabla);
void readAndWriteBloque(char* listaDeBloques);

#endif /* COMPACTADOR_H_ */
