/*
 * Bitmap.h
 *
 *  Created on: 9 jul. 2019
 *      Author: juanmaalt
 */

#ifndef ADHOC_BITMAP_H_
#define ADHOC_BITMAP_H_

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include "../Lissandra.h"

/*GLOBALES*/
sem_t leyendoBitarray;

/*FUNCIONES*/
void leerBitmap();
char* getBloqueLibre();
void liberarBloque(int pos);
void escribirBitmap();
void actualizarBitarray();

#endif /* ADHOC_BITMAP_H_ */
