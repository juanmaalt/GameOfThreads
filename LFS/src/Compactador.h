
#ifndef COMPACTADOR_H_
#define COMPACTADOR_H_

#include "Lissandra.h"
#include "../APILissandra.h"

void compactarTabla(char* nombreTabla);


int isTmp(char* name) ;

char* extension(char *filename);

void compactarArchivo(FILE* file) ;

Registro* registro(char* line, size_t len);

void compactarRegistro(Registro* registro) ;

void deleteFile(char* fileName);

#endif /* COMPACTADOR_H_ */
