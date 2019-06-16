/*
 * Metrics.c
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */

#include "Metrics.h"

//FUNCIONES: Privadas
static void *servicio_metricas(void *null);
static int iniciar_servicio_metricas();
static int mostrar_metricas_por_pantalla();

int ver_metricas(){
	if(servicioEstaEncendido)
		return mostrar_metricas_por_pantalla();

	if(iniciar_servicio_metricas() == EXIT_FAILURE)
		return EXIT_FAILURE;

	return ver_metricas();
}





void no_ver_metricas(){
	pthread_cancel(servicioMetricas);
}





static void *servicio_metricas(void *null){
	INICIO:
	sleep(TIEMPO_DE_REFRESCO_DE_METRICAS);
	mostrar_metricas_por_pantalla();
	goto INICIO;
	return NULL;
}







static int iniciar_servicio_metricas(){
	metricas.At.EventualConsistency.memoryLoad = dictionary_create();
	metricas.At.HashStrongConsistency.memoryLoad = dictionary_create();
	metricas.At.StrongConsistency.memoryLoad = dictionary_create();

	if(pthread_create(&servicioMetricas, NULL, servicio_metricas, NULL))
		RETURN_ERROR("Metricas.c: iniciar_servicio_metricas: fallo la creacion del servicio de metricas");

	servicioEstaEncendido=1;
	return EXIT_SUCCESS;
}





static int mostrar_metricas_por_pantalla(){
	if(metricas.At.EventualConsistency.memoryLoad == NULL || metricas.At.HashStrongConsistency.memoryLoad == NULL || metricas.At.StrongConsistency.memoryLoad == NULL){
		RETURN_ERROR("Metrics.c: mostrar_metricas: no se pueden mostrar por que el servicio no se inicializo");
	}

	void mostrar_elemento_dictionary(char* key, void* value){
		log_info(logger_visible, "Memoria: %s, Loads: int", key, *(int *)value);
	}

	//Logger visible

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

	//Logger invisible
	//TODO: mostrar metricas invisibles

	return EXIT_SUCCESS;
}
