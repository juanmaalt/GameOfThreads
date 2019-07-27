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
#include "Consola.h"
#include <stdlib.h>

void cambiarNombreFilesTemp(char* pathTabla);
char* obtenerListaDeBloques(int particion, char* nombreTabla);
char* firstBloqueDisponible(char* listaDeBloques);
int caracteresEnBloque(char* bloque);
//void escribirLinea(char* bloque, char* linea, char* nombreTabla, int particion);
void escribirEnBloque(char* bloque, char* linea);
bool esRegistroMasReciente(timestamp_t timestamp, int key, char* listaDeBloques);
void borrarArchivosTmpc(char* nombreTabla);
void procesarPeticionesPendientes(char *nombreTabla);
void destruirPeticionesPendientes(char *nombreTabla);
int getMin(int value1, int value2);

void sacarTablaDeDiccCompactacion(char* nombreTabla);

char **generarRegistroBloque(t_list *registros);


#endif /* ADHOC_FUNCIONESCOMP_H_ */
