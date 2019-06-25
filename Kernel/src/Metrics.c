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
static void refrescar_valores(); //Como los valores que se deben mostrar son cada 30 segundos, entonces cada 30 segundos tengo que reestablecer todos los valores
static int servicioEstaEncendido;

int ver_metricas(){
	if(servicioEstaEncendido)
		return mostrar_metricas_por_pantalla();

	if(iniciar_servicio_metricas() == EXIT_FAILURE)
		return EXIT_FAILURE;

	return ver_metricas();
}





void no_ver_metricas(){
	servicioEstaEncendido = 0;
	pthread_cancel(servicioMetricas);
}





static void *servicio_metricas(void *null){
	INICIO:
	//refrescar_valores(); TODO: descomentar para la entrega final
	sleep(TIEMPO_DE_REFRESCO_DE_METRICAS);
	mostrar_metricas_por_pantalla();
	goto INICIO;
	return NULL;
}







static int iniciar_servicio_metricas(){
	if(memoriasExistentes == NULL)
		RETURN_ERROR("Metrics.c: iniciar_servicio_metricas: deberia esperar a que se inicien las estructuras de las memorias");

	if(pthread_create(&servicioMetricas, NULL, servicio_metricas, NULL))
		RETURN_ERROR("Metricas.c: iniciar_servicio_metricas: fallo la creacion del servicio de metricas");

	servicioEstaEncendido=1;
	return EXIT_SUCCESS;
}





static int mostrar_metricas_por_pantalla(){
	if(memoriasExistentes == NULL){
		RETURN_ERROR("Metrics.c: mostrar_metricas: deberia esperar a que se inicien las estructuras de las memorias");
	}

	//Logger visible

	printf("\n");

	log_info(logger_visible, GRN"StrongConsistency"STD);
	log_info(logger_visible, "Read latency: %llu", metricas.At.StrongConsistency.readLatency);
	log_info(logger_visible, "Write latency: %llu", metricas.At.StrongConsistency.writeLatency);
	log_info(logger_visible, "Reads: %d", metricas.At.StrongConsistency.reads);
	log_info(logger_visible, "Writes: %d", metricas.At.StrongConsistency.writes);

	printf("\n");

	log_info(logger_visible, GRN"HashStrongConsistency"STD);
	log_info(logger_visible, "Read latency: %llu", metricas.At.HashStrongConsistency.readLatency);
	log_info(logger_visible, "Write latency: %llu", metricas.At.HashStrongConsistency.writeLatency);
	log_info(logger_visible, "Reads: %d", metricas.At.HashStrongConsistency.reads);
	log_info(logger_visible, "Writes: %d", metricas.At.HashStrongConsistency.writes);

	printf("\n");

	log_info(logger_visible, GRN"EventualConsistency"STD);
	log_info(logger_visible, "Read latency: %llu", metricas.At.EventualConsistency.readLatency);
	log_info(logger_visible, "Write latency: %llu", metricas.At.EventualConsistency.writeLatency);
	log_info(logger_visible, "Reads: %d", metricas.At.EventualConsistency.reads);
	log_info(logger_visible, "Writes: %d", metricas.At.EventualConsistency.writes);

	printf("\n");

	//Logger invisible
	//TODO: mostrar metricas invisibles

	return EXIT_SUCCESS;
}





static void refrescar_valores(){
	//TODO: refrescar valores de las memorias y otras cosas mas q se agregaron
	metricas.At.EventualConsistency.readLatency = 0;
	metricas.At.EventualConsistency.reads = 0;
	metricas.At.EventualConsistency.writeLatency = 0;
	metricas.At.EventualConsistency.writes = 0;

	metricas.At.HashStrongConsistency.readLatency = 0;
	metricas.At.HashStrongConsistency.reads = 0;
	metricas.At.HashStrongConsistency.writeLatency = 0;
	metricas.At.HashStrongConsistency.writes = 0;

	metricas.At.StrongConsistency.readLatency = 0;
	metricas.At.StrongConsistency.reads = 0;
	metricas.At.StrongConsistency.writeLatency = 0;
	metricas.At.StrongConsistency.writes = 0;
}
