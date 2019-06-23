/*
 * APILissandra.h
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#ifndef APILISSANDRA_H_
#define APILISSANDRA_H_

#include "../Lissandra.h"
#include "Metadata.h"
#include <parser/compresor_describe.h>

//GLOBALES
Metadata_tabla metadata;

//API
Operacion selectAPI(Comando comando);
Operacion insertAPI(Comando comando);
Operacion createAPI(Comando);
Operacion describeAPI(Comando);
Operacion dropAPI(Comando);

//COMPLEMENTARIAS
bool existeTabla(char* key);
int getMetadata(char* nombreTabla, t_config* metadataFile);
t_config* leerMetadata(char* nombreTabla);
void extraerMetadata(t_config* metadataFile);
t_list* getData(char* nombreTabla);
int calcularParticionNbr(char* key, int particiones);
void buscarValue(t_list* data, t_list* listaDeValues, char* key, int particionNbr);
t_list* buscarValueEnLista(t_list* data, char* key);
Operacion getValueMasReciente(t_list* lista);
timestamp_t checkTimestamp(char* timestamp);
void crearTablaEnMemtable(char* nombreTabla);
void setPathTabla(char* path, char* nombreTabla);
void crearDirectorioTabla(char* nombreTabla);
void crearArchivo(char* path, char* nombre);
void escribirArchivoMetadata(char* path, Comando comando);
void crearArchivosBinarios(char* path, int particiones);
void getStringDescribe(char* path, char* pathMetadata, char* string, char* nombreTabla, Operacion *resultadoDescribe);

//AD-HOC
void mostrarMetadata();
void recorrerTabla(t_list* lista);
void insertInFile(char* path, int particionNbr, char* key, char* value);

#endif /* APILISSANDRA_H_ */
