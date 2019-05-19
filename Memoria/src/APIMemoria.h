/*
 * APIMemoria.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef APIMEMORIA_H_
#define APIMEMORIA_H_

#include "Memoria.h"

typedef struct segmento segmento_t;
typedef struct pagina pagina_t;

//Funciones complementarias
segmento_t* verificarExistenciaSegmento(char*);

bool contieneKey(segmento_t*,uint16_t,char*);

//API	TODO: valores de retorno Resultado
void selectAPI(char*, Comando);
void insertAPI(char*,Comando);
void create(char*,Comando);
void describeAPI();
void dropAPI();
void journalAPI();

#endif /* APIMEMORIA_H_ */
