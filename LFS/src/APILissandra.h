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
Operacion selectAPI(Comando comando);
Operacion insertAPI(Comando comando);
Operacion createAPI(Comando);
void describeAPI(Comando);
void dropAPI(Comando);

//COMPLEMENTARIAS
bool existeTabla(char* key);
int getMetadata(char* nombreTabla, t_config* metadataFile);
t_config* leerMetadata(char* nombreTabla);
void extraerMetadata(t_config* metadataFile);
t_list* getData(char* nombreTabla);
int calcularParticionNbr(char* key, int particiones);
t_list* buscarValue(t_list* data, char* key, int particionNbr);
Operacion getValueMasReciente(t_list* lista);
timestamp_t checkTimestamp(char* timestamp);
void crearTablaEnMemtable(char* nombreTabla);
void setPathTabla(char* path, char* nombreTabla);
void crearDirectorioTabla(char* nombreTabla);
void crearArchivo(char* path, char* nombre);
void escribirArchivoMetadata(char* path, Comando comando);
void crearArchivosBinarios(char* path, int particiones);


//AD-HOC
void mostrarMetadata();
void recorrerTabla(t_list* lista);
void insertInFile(char* path, int particionNbr, char* key, char* value);

#endif /* APILISSANDRA_H_ */
