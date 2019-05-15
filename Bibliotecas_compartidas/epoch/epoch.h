/*
 * epoch.h
 *
 *  Created on: 15 may. 2019
 *      Author: facusalerno
 */

#ifndef EPOCH_EPOCH_H_
#define EPOCH_EPOCH_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

typedef unsigned long long timestamp_t;



	/**
	* @NAME: getCurrentTime
	* @DESC: devuelve el tiempo en milisegundos en el instante actual
	*/
	timestamp_t getCurrentTime();

	/**
	* @NAME: seeCurrentTime
	* @DESC: muestra por pantalla el tiempo en milisegundos en el instante actual
	*/
	void seeCurrentTime();

#endif /* EPOCH_EPOCH_H_ */
