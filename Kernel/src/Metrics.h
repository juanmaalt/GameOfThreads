/*
 * Metrics.h
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */

#ifndef METRICS_H_
#define METRICS_H_
#define TIEMPO_DE_REFRESCO_DE_METRICAS 30

#include "Kernel.h"
#include <commons/collections/dictionary.h>

typedef timestamp_t timeEpoch;

struct valores_de_metricas{
	enum{
		StrongConsistency,
		HashStrongConsistency,
		EventualConsistency,
	}Criterio;
	struct{
		struct{
			timeEpoch readLatency;
			timeEpoch writeLatency;
			timeEpoch acumuladorTiemposRead; //Para el promedio, si no no tengo forma de saber el readLatency o el otro
			timeEpoch acumuladorTiemposWrite;
			int reads;
			int writes;
		}StrongConsistency;
		struct{
			timeEpoch readLatency;
			timeEpoch writeLatency;
			timeEpoch acumuladorTiemposRead;
			timeEpoch acumuladorTiemposWrite;
			int reads;
			int writes;
		}HashStrongConsistency;
		struct{
			timeEpoch readLatency;
			timeEpoch writeLatency;
			timeEpoch acumuladorTiemposRead;
			timeEpoch acumuladorTiemposWrite;
			int reads;
			int writes;
		}EventualConsistency;
	}At;
};

typedef struct valores_de_metricas Metricas;

//GLOBALES: Metricas
Metricas metricas;

//FUNCIONES: Publicas
int ver_metricas(); //Comienza el servicio de metricas que se loggean cada 30 segundos. Ademas se muestran cada vez que se invoca
void no_ver_metricas();

#endif /* METRICS_H_ */
