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
#include "../Lissandra.h"

void leerBitmap();
int getBloqueLibre();
void liberarBloque(int pos);
void escribirBitmap();
void binarioATexto(char* binario, char* texto, int cantSimbolos);
unsigned long binarioADecimal(char* binario, int length);
void actualizarBitarray();

#endif /* ADHOC_BITMAP_H_ */
