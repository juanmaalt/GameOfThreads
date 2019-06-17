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

void checkEstructuraFS();
void crearEstructuraFS(int blockSize, int blocks, char* magicNumber);
int crearDirectorio(char* path);
void crearDirectorioDeMontaje(char* puntoMontaje);
void crearBloques(char* path, int blocks);

#endif /* FILESYSTEM_H_ */
