/*
 * FuncionesAPI.h
 *
 *  Created on: 23 jun. 2019
 *      Author: juanmaalt
 */

#ifndef ADHOC_FUNCIONESAPI_H_
#define ADHOC_FUNCIONESAPI_H_

#include "../Lissandra.h"
#include "Metadata.h"
#include <parser/compresor_describe.h>

//GLOBALES
Metadata_tabla metadata;

//COMPLEMENTARIAS
bool existeTabla(char* key);
int getMetadata(char* nombreTabla, t_config* metadataFile);
t_config* leerMetadata(char* nombreTabla);
void extraerMetadata(t_config* metadataFile);
t_list* getData(char* nombreTabla);
int calcularParticionNbr(char* key, int particiones);
void buscarValueEnFiles(char* nombreTabla, char* key, int particionNbr, t_list* listaDeValues);
void leerTemps(char* nombreTabla, char* key, t_list* listaDeValues);
t_list* buscarValueEnLista(t_list* data, char* key);
void getValueMasReciente(t_list* lista, Operacion* resultadoSelect);
timestamp_t checkTimestamp(char* timestamp);
void crearTablaEnMemtable(char* nombreTabla);
void crearDirectorioTabla(char* nombreTabla);
void crearArchivo(char* path, char* nombre);
void escribirArchivoMetadata(char* path, Comando comando);
void crearArchivosBinarios(char* path, int particiones);
void getStringDescribe(char* path, char* string, char* nombreTabla, Operacion *resultadoDescribe);
int removerDirectorio(char *path);
void limpiarBloquesEnBitarray(char* nombreTabla);
int iniciarCompactacion(char* nombreTabla);

//AD-HOC
void mostrarMetadata();
void recorrerTabla(t_list* lista);
void insertInFile(char* path, int particionNbr, char* key, char* value);


#endif /* ADHOC_FUNCIONESAPI_H_ */
