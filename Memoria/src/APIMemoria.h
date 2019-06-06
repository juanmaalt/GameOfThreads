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

//API	TODO: valores de retorno Resultado
Operacion ejecutarOperacion(char*);

Operacion selectAPI(char*, Comando);
Operacion insertAPI(char*,Comando);
Operacion createAPI(char*,Comando);
Operacion describeAPI();
Operacion dropAPI();
Operacion journalAPI();

#endif /* APIMEMORIA_H_ */
