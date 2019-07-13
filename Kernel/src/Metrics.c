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
	refrescar_valores();
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

	void mostrar_estadisticas_de_memorias(void *memoria){
		printf("\n");
			if(((Memoria*)memoria)->fueAsociada){
				log_info(logger_visible, "Memoria %d %s: %s", ((Memoria*)memoria)->numero, ((Memoria*)memoria)->ip, ((Memoria*)memoria)->puerto);
				if(((Memoria*)memoria)->Metrics.SC.estaAsociada)
					log_info(logger_visible, "Asignada al criterio SC | INSERTS: %d | SELECTS: %d", ((Memoria*)memoria)->Metrics.SC.cantidadInsert, ((Memoria*)memoria)->Metrics.SC.cantidadSelect);
				if(((Memoria*)memoria)->Metrics.SHC.estaAsociada)
					log_info(logger_visible, "Asignada al criterio HSC | INSERTS: %d | SELECTS: %d", ((Memoria*)memoria)->Metrics.SHC.cantidadInsert, ((Memoria*)memoria)->Metrics.SHC.cantidadSelect);
				if(((Memoria*)memoria)->Metrics.EC.estaAsociada)
					log_info(logger_visible, "Asignada al criterio EC | INSERTS: %d | SELECTS: %d", ((Memoria*)memoria)->Metrics.EC.cantidadInsert, ((Memoria*)memoria)->Metrics.EC.cantidadSelect);
			}else{
				log_info(logger_visible, "Memoria %d %s:%s no esta asociada a ningun criterio", ((Memoria*)memoria)->numero, ((Memoria*)memoria)->ip, ((Memoria*)memoria)->puerto);
			}
	}

	//Logger visible

	printf("\n");

	log_info(logger_visible, GRN"Operaciones totales: %d"STD, metricas.operacionesTotales);

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

	if(!list_is_empty(memoriasExistentes)){
		log_info(logger_visible, GRN"Memorias"STD);
		list_iterate(memoriasExistentes, mostrar_estadisticas_de_memorias);
	}

	//Logger invisible
	//TODO: mostrar metricas invisibles

	return EXIT_SUCCESS;
}





static void refrescar_valores(){
	void refrescar_memoria(void *memoria){
		((Memoria*)memoria)->Metrics.EC.cantidadInsert = 0;
		((Memoria*)memoria)->Metrics.EC.cantidadSelect = 0;
		((Memoria*)memoria)->Metrics.SC.cantidadInsert = 0;
		((Memoria*)memoria)->Metrics.SC.cantidadSelect = 0;
		((Memoria*)memoria)->Metrics.SHC.cantidadInsert = 0;
		((Memoria*)memoria)->Metrics.SHC.cantidadSelect = 0;
	}
	list_iterate(memoriasExistentes, refrescar_memoria);

	metricas.operacionesTotales = 0;

	metricas.At.EventualConsistency.readLatency = 0;
	metricas.At.EventualConsistency.acumuladorTiemposRead = 0;
	metricas.At.EventualConsistency.acumuladorTiemposWrite = 0;
	metricas.At.EventualConsistency.reads = 0;
	metricas.At.EventualConsistency.writeLatency = 0;
	metricas.At.EventualConsistency.writes = 0;

	metricas.At.HashStrongConsistency.readLatency = 0;
	metricas.At.HashStrongConsistency.acumuladorTiemposRead = 0;
	metricas.At.HashStrongConsistency.acumuladorTiemposWrite = 0;
	metricas.At.HashStrongConsistency.reads = 0;
	metricas.At.HashStrongConsistency.writeLatency = 0;
	metricas.At.HashStrongConsistency.writes = 0;

	metricas.At.StrongConsistency.readLatency = 0;
	metricas.At.StrongConsistency.acumuladorTiemposRead = 0;
	metricas.At.StrongConsistency.acumuladorTiemposWrite = 0;
	metricas.At.StrongConsistency.reads = 0;
	metricas.At.StrongConsistency.writeLatency = 0;
	metricas.At.StrongConsistency.writes = 0;
}
