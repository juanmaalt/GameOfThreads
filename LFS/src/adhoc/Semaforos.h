/*
 * Semaforos.h
 *
 *  Created on: 31 jul. 2019
 *      Author: facusalerno
 */

#ifndef ADHOC_SEMAFOROS_H_
#define ADHOC_SEMAFOROS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <epoch/epoch.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "../Lissandra.h"

	/*========== Funciones publicas ==========*/

	/**
	* @NAME: iniciar_semaforos
	* @DESC: inicia lista y semaforo mutex de todos los tipos de semaforos para poder usarlos
	*/
	int iniciar_semaforos(void);





	/*========== Mutex generales ==========*/

	sem_t mutexMemtable;
	sem_t mutexAgregarTablaMemtable;
	sem_t mutexVConfig;


	/*========== Estructura semaforos de compactacion ==========*/
	/*(Para bloquear acceso a nuevas request)*/

	typedef struct semt_comp{
		char* tabla;
		int peticionesEnEspera;
		int peticionesEnEsperaSelect;
		sem_t semaforoGral;
		sem_t semaforoSelect;
		pthread_t compactacionService;
		timestamp_t inicioBloqueo;
		timestamp_t finBloqueo;
	}SemaforoCompactacion;

	sem_t mutexPeticionesPorTabla;

	t_list* semaforosPorTabla;

	/*Semaforos de compactacion*/

	/**
	* @NAME: bloquearTabla
	* @DESC: bloquea una tabla para poder compactarlo evitando que nuevas request entren
	*/
	void bloquearTabla(char *tabla);

	/**
	* @NAME: desbloquearTabla
	* @DESC: desbloquea la tabla analogamente
	*/
	void desbloquearTabla(char *tabla);

	/**
	* @NAME: bloquearSelect
	* @DESC: igual que bloquearTabla pero para select. las funciones hacen casi lo mismo, la diferencia es donde se
	* 		 ponen y su expresivo nombre que indica donde deberian ponerse.
	*/
	void bloquearSelect(char *tabla);

	/**
	* @NAME: desbloquearSelect
	* @DESC: desbloquearSelect
	*/
	void desbloquearSelect(char *tabla);

	/**
	* @NAME: tryExecute
	* @DESC: se fija si la tabla esta en compactacion. si no esta en compactacion, continua su ejecucion. caso contrario
	* 		 se queda esperando hasta que se llame a desbloquearTabla
	*/
	void tryExecute(char *tabla);

	/**
	* @NAME: tryExecuteSelect
	* @DESC: tryExecuteSelect analogo tryExecute
	*/
	void tryExecuteSelect(char *tabla);

	/**
	* @NAME: reiniciarSemaforos
	* @DESC: deberia llamarse siemore que se desbloquea una tabla
	*/
	void reiniciarSemaforos(char *tabla);

	/**
	* @NAME: tomarTiempoInicio
	* @DESC: tomarTiempoInicio
	*/
	void tomarTiempoInicio(char *tabla);

	/**
	* @NAME: tomarTiempoInicio
	* @DESC: tomarTiempoInicio
	*/
	void tomarTiempoFin(char *tabla);

	/**
	* @NAME: loggearTiempoCompactacion
	* @DESC: si se tomo el tiempo de inicio y de fin, entonces se puede llamar a esta funcion sin problemas
	*/
	void loggearTiempoCompactacion(char *tabla);





	/*========== Estructura semaforo de request activas ==========*/
	/*(Para que la compactacion no inicie si hay request ejecutandose ahora)*/

	typedef struct semt_ex_request{
		char *tabla;
		int peticionesActivasSelect;
		sem_t semaforoSelect;
	}SemaforoRequestActivas;

	sem_t mutexRequestActivas;

	t_list *requestActivas;

	/**
	* @NAME: esperarRequestActivas
	* @DESC: espera a que no haya ninguna request ejecutandose sobre la tabla
	*/
	void esperarSelectsActivos(char *tabla);

	/**
	* @NAME: operacionTerminada
	* @DESC: deberia llamarse despues de esperarRequestActivas
	*/
	void operacionTerminadaForSelects(char *tabla);

	/**
	* @NAME: seVaAEjecutarRequest
	* @DESC: notifica al semaforo que se va a ejecutar una request
	*/
	void seVaAEjecutarRequestSelect(char *tabla);

	/**
	* @NAME: seTerminoDeEjecutarRequest
	* @DESC: notifica al semaforo que ya se termino de ejecutar la request
	*/
	void seTerminoDeEjecutarRequestSelect(char *tabla);

#endif /* ADHOC_SEMAFOROS_H_ */
