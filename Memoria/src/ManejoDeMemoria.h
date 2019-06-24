/*
 * ManejoDeMemoria.h
 *
 *  Created on: 23 jun. 2019
 *      Author: fdalmaup
 */

#ifndef MANEJODEMEMORIA_H_
#define MANEJODEMEMORIA_H_

#include "Memoria.h"

typedef struct segmento segmento_t;
typedef struct pagina pagina_t;
typedef struct registroTablaPag registroTablaPag_t;
typedef struct tabla_de_paginas tabla_de_paginas_t;

char* obtenerPath(segmento_t* segmento);

void mostrarContenidoMemoria(void);

void asignarPathASegmento(segmento_t * , char* );

int colocarPaginaEnMemoria(timestamp_t , uint16_t , char* );

void crearRegistroEnTabla(tabla_de_paginas_t *, int ,bool);

Operacion tomarContenidoPagina(registroTablaPag_t);

int hayPaginaDisponible(void);

void insertarPaginaDeSegmento(char* , uint16_t , timestamp_t, segmento_t * , bool); //bool para indicar se es INSERT o SELECT

void actualizarValueDeKey(char *, registroTablaPag_t *);

void crearSegmentoInsertandoRegistro(char * , char*, timestamp_t , uint16_t, bool); //bool para indicar se es INSERT o SELECT



#endif /* MANEJODEMEMORIA_H_ */
