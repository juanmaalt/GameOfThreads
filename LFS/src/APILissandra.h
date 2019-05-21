/*
 * APILissandra.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef APILISSANDRA_H_
#define APILISSANDRA_H_

#include "Lissandra.h"
#include "Metadata.h"

//API
int selectAPI(Comando comando);
void insertAPI(Comando comando);
void createAPI(Comando);
void describeAPI(Comando);
void dropAPI(Comando);

bool existeTabla(char* key);
t_list* getData(char* nombreTabla);
Metadata_tabla* getMetadataValues(t_list* data);
int calcularParticionNbr(char* key, int particiones);

t_list* buscarValue(t_list* data, char* key, int particionNbr);

#endif /* APILISSANDRA_H_ */
