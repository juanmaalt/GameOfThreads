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
typedef struct registroTablaPag registroTablaPag_t;



//Funciones complementarias
bool verificarExistenciaSegmento(char*,segmento_t**);

bool contieneKey(segmento_t* , uint16_t , registroTablaPag_t **);

Operacion tomarContenidoPagina(registroTablaPag_t);

int hayPaginaDisponible(void);

void insertarPaginaDeSegmento(char* , uint16_t , segmento_t * );

//Comunicacion con FS
Operacion recibirRequestFS(void);
void enviarRequestFS(char* );

Operacion ejecutarOperacion(char*, bool);

Operacion selectAPI(char*, Comando);
Operacion insertAPI(char*,Comando);
Operacion createAPI(char*,Comando);
Operacion describeAPI();
Operacion dropAPI(char* input, Comando comando);
Operacion journalAPI();

#endif /* APIMEMORIA_H_ */
