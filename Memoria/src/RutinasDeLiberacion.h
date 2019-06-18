/*
 * RutinasDeLiberacion.h
 *
 *  Created on: 15 jun. 2019
 *      Author: fdalmaup
 */

#ifndef RUTINASDELIBERACION_H_
#define RUTINASDELIBERACION_H_

#include "Memoria.h"

void liberarRecursos();
void liberarMCBs(void* MCBAdestruir);
void liberarRegistroTablaPags(void* registroAdestruir);
void liberarSegmento(void* segmentoAdestruir);

#endif /* RUTINASDELIBERACION_H_ */
