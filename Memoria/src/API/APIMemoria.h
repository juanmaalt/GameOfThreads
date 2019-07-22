/*
 * APIMemoria.h
 *
 *  Created on: 15 may. 2019
 *      Author: utnso
 */

#ifndef APIMEMORIA_H_
#define APIMEMORIA_H_

#include "../Memoria.h"
#include "UtilitariasAPI.h"
#include "../ManejoDeMemoria.h"
#include "../ComunicacionFS.h"

typedef struct segmento segmento_t;
typedef struct pagina pagina_t;
typedef struct registroTablaPag registroTablaPag_t;

void loggearMemoria(void);

Operacion ejecutarOperacion(char*, bool);


Operacion selectAPI(char*, Comando);
Operacion insertAPI(char*,Comando);
Operacion createAPI(char*,Comando);
Operacion describeAPI(char* input, Comando comando);
Operacion dropAPI(char* input, Comando comando);
Operacion journalAPI();

#endif /* APIMEMORIA_H_ */
