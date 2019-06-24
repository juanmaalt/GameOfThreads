/*
 * ComunicacionFS.h
 *
 *  Created on: 24 jun. 2019
 *      Author: fdalmaup
 */

#ifndef COMUNICACIONFS_H_
#define COMUNICACIONFS_H_

#include "Memoria.h"

//Globales obtenidas despues de handshake
int	tamanioValue;
char *pathLFS;


int conectarLFS();
int handshakeLFS(int socketLFS);

Operacion recibirRequestFS(void);
void enviarRequestFS(char* );


#endif /* COMUNICACIONFS_H_ */
