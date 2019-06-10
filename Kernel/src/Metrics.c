/*
 * Metrics.c
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */

#include "Metrics.h"

void *servicio_metricas(void *null){
	INICIO:
	mostrar_metricas();
	sleep(30);
	goto INICIO;
	return NULL;
}

int iniciar_servicio_metricas(){
	metricas.At.EventualConsistency.memoryLoad = dictionary_create();
	metricas.At.HashStrongConsistency.memoryLoad = dictionary_create();
	metricas.At.StrongConsistency.memoryLoad = dictionary_create();

	if(pthread_create(&servicioMetricas, NULL, servicio_metricas, NULL))
		RETURN_ERROR("Metricas.c: iniciar_servicio_metricas: fallo la creacion del servicio de metricas");

	return EXIT_SUCCESS;
}

void mostrar_metricas(){
	if(metricas.At.EventualConsistency.memoryLoad == NULL || metricas.At.HashStrongConsistency.memoryLoad == NULL || metricas.At.StrongConsistency.memoryLoad == NULL){
		log_error(logger_error, "Metrics.c: mostrar_metricas: no se pueden mostrar por que el servicio no se inicializo");
		return;
	}

	void mostrar_elemento_dictionary(char* key, void* value){
		log_info(logger_visible, "Memoria: %s, Loads: int", key, *(int *)value);
	}

	printf("\n");

	log_info(logger_visible, GRN"StrongConsistency"STD);
	log_info(logger_visible, "Read latency: %llu", metricas.At.StrongConsistency.readLatency);
	log_info(logger_visible, "Write latency: %llu", metricas.At.StrongConsistency.writeLatency);
	log_info(logger_visible, "Reads: %d", metricas.At.StrongConsistency.reads);
	log_info(logger_visible, "Writes: %d", metricas.At.StrongConsistency.writes);
	dictionary_iterator(metricas.At.StrongConsistency.memoryLoad, mostrar_elemento_dictionary);

	printf("\n");

	log_info(logger_visible, GRN"HashStrongConsistency"STD);
	log_info(logger_visible, "Read latency: %llu", metricas.At.HashStrongConsistency.readLatency);
	log_info(logger_visible, "Write latency: %llu", metricas.At.HashStrongConsistency.writeLatency);
	log_info(logger_visible, "Reads: %d", metricas.At.HashStrongConsistency.reads);
	log_info(logger_visible, "Writes: %d", metricas.At.HashStrongConsistency.writes);
	dictionary_iterator(metricas.At.HashStrongConsistency.memoryLoad, mostrar_elemento_dictionary);\

	printf("\n");

	log_info(logger_visible, GRN"EventualConsistency"STD);
	log_info(logger_visible, "Read latency: %llu", metricas.At.EventualConsistency.readLatency);
	log_info(logger_visible, "Write latency: %llu", metricas.At.EventualConsistency.writeLatency);
	log_info(logger_visible, "Reads: %d", metricas.At.EventualConsistency.reads);
	log_info(logger_visible, "Writes: %d", metricas.At.EventualConsistency.writes);
	dictionary_iterator(metricas.At.EventualConsistency.memoryLoad, mostrar_elemento_dictionary);

	printf("\n");
}
