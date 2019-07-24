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

typedef struct dirent EntradaDirectorio;
typedef struct registro_t Registro;

//GLOBALES
//Metadata_tabla metadata;

//NUEVAS
int directory_iterate(char *pathDirectorio, void(*closure)(EntradaDirectorio *));
int directory_iterate_if(char *pathDirectorio, bool (*condicion)(EntradaDirectorio*), void(*closure)(EntradaDirectorio *));
bool directory_any_satisfy(char *pathDirectorio, bool(*closure)(EntradaDirectorio *));
int dump_iterate_registers(char *pathFile, char *mode, void(*closure)(Registro*));

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
