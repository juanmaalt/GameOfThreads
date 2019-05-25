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

//GLOBALES
Metadata_tabla metadata;

//API
int selectAPI(Comando comando);
int insertAPI(Comando comando);
int createAPI(Comando);
void describeAPI(Comando);
void dropAPI(Comando);

//COMPLEMENTARIAS
bool existeTabla(char* key);
t_config* leerMetadata(char* nombreTabla);
void extraerMetadata(t_config* metadataFile);
t_list* getData(char* nombreTabla);
int calcularParticionNbr(char* key, int particiones);
t_list* buscarValue(t_list* data, char* key, int particionNbr);
void getValueMasReciente(t_list* lista);
timestamp_t checkTimestamp(char* timestamp);
void crearTablaEnMemtable(char* nombreTabla);

//AD-HOC
void mostrarMetadata();
void recorrerTabla(t_list* lista);

#endif /* APILISSANDRA_H_ */
