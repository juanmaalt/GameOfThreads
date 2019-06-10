/*
 * Metrics.h
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */

#ifndef METRICS_H_
#define METRICS_H_

#include "Kernel.h"
#include <commons/collections/dictionary.h>

typedef timestamp_t timeEpoch;

struct valores_de_metricas{
	enum{
		StrongConsistency,
		HashStrongConsistency,
		EventualConsistency,
	}Criterio;
	union{
		struct{
			timeEpoch readLatency;
			timeEpoch writeLatency;
			int reads;
			int writes;
			t_dictionary *memoryLoad; //Lista de pares ordenados [memoria; cantidad insert+cantidad select]
		}StrongConsistency;
		struct{
			timeEpoch readLatency;
			timeEpoch writeLatency;
			int reads;
			int writes;
			t_dictionary *memoryLoad;
		}HashStrongConsistency;
		struct{
			timeEpoch readLatency;
			timeEpoch writeLatency;
			int reads;
			int writes;
			t_dictionary *memoryLoad;
		}EventualConsistency;
	}At;
};

typedef struct valores_de_metricas Metricas;

//GLOBALES: Metricas
Metricas metricas;

//FUNCIONES: Publicas
int iniciar_servicio_metricas();
void mostrar_metricas();
void finalizar_servicio_metricas();

#endif /* METRICS_H_ */
