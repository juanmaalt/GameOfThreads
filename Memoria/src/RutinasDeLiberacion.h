/*
 * RutinasDeLiberacion.h
 *
 *  Created on: 15 jun. 2019
 *      Author: fdalmaup
 */

#ifndef RUTINASDELIBERACION_H_
#define RUTINASDELIBERACION_H_

#include "Memoria.h"

typedef struct segmento segmento_t;

void liberarRecursos();
void liberarMCBs(void* MCBAdestruir);
void liberarRegistroTablaPags(void* registroAdestruir);
void liberarSegmento(void* segmentoAdestruir);

void removerSegmentoDeTabla(segmento_t* );

#endif /* RUTINASDELIBERACION_H_ */
