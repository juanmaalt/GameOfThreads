/*
 * FileSystem.h
 *
 *  Created on: 16 jun. 2019
 *      Author: juanmaalt
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Lissandra.h"

/*ESTRUCTURAS*/
typedef struct {
	int blockSize;
	int blocks;
	char* magicNumber;
}Metadata_FS;

/*GLOBALES*/
Metadata_FS metadataFS;
t_config* metadata_FS;

/*FUNCIONES*/
void checkEstructuraFS();
void crearEstructuraFS(int blockSize, int blocks, char* magicNumber);
int crearDirectorio(char* path);
void crearDirectorioDeMontaje(char* puntoMontaje);
void crearBloques(char* path, int blocks);
void crearMetadata(char* path ,int blockSize, int blocks, char* magicNumber);
void checkExistenciaDirectorio(char* path, char* carpeta);
void checkDirectorios();
void levantarTablasExistentes();
void agregarBloqueEnParticion(char* bloque, char* nombreTabla, int particion);
void agregarBloqueEnBitarray(char* nombreCarpeta);


int levantarMetadata();
t_config* leer_MetadataFS();
void extraer_MetadataFS();

#endif /* FILESYSTEM_H_ */
