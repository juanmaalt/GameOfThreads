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
void compactar(char* nombreTabla);
void leerTemporal(char* pathTemp, int particiones, char* nombreTabla);
char* obtenerListaDeBloques(int particion, char* nombreTabla);
char* firstBloqueDisponible(char* listaDeBloques);
int caracteresEnBloque(char* pathBloque, char* pathBloques, char* bloque);
void escribirEnBloque(char* bloque, char* linea);

#endif /* COMPACTADOR_H_ */
