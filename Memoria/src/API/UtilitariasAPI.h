/*
 * UtilitariasAPI.h
 *
 *  Created on: 23 jun. 2019
 *      Author: fdalmaup
 */

#ifndef API_UTILITARIASAPI_H_
#define API_UTILITARIASAPI_H_

#include "../Memoria.h"
#include "APIMemoria.h"

typedef struct segmento segmento_t;
typedef struct pagina pagina_t;
typedef struct registroTablaPag registroTablaPag_t;


bool verificarExistenciaSegmento(char*,segmento_t**);

bool contieneKey(segmento_t* , uint16_t , registroTablaPag_t **);



#endif /* API_UTILITARIASAPI_H_ */
