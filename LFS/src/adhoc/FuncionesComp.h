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
#include <stdlib.h>

void cambiarNombreFilesTemp(char* pathTabla);
void leerTemporal(char* pathTemp, int particiones, char* nombreTabla);
char* obtenerListaDeBloques(int particion, char* nombreTabla);
char* firstBloqueDisponible(char* listaDeBloques);
int caracteresEnBloque(char* bloque);
void escribirLinea(char* bloque, char* linea, char* nombreTabla, int particion);
void escribirEnBloque(char* bloque, char* linea);
bool esRegistroMasReciente(int timestamp, int key, char* listaDeBloques);
void procesarPeticionesPendientes(char *nombreTabla);

void agregarTablaEnDiccCompactacion(char* nombreTabla);
void sacarTablaDeDiccCompactacion(char* nombreTabla);


#endif /* ADHOC_FUNCIONESCOMP_H_ */
