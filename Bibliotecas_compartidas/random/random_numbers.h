/*
 * random_numbers.h
 *
 *  Created on: 13 jun. 2019
 *      Author: facusalerno
 */

#ifndef RANDOM_RANDOM_NUMBERS_H_
#define RANDOM_RANDOM_NUMBERS_H_

#define RANGO_MAX 99999999
#define RANGO_MIN 10000000

	typedef unsigned int id;

	#include <stdlib.h>

	/**
	* @NAME: getNumber
	* @DESC: devuelve un numero entero positivo random de 8 digitos
	*/
	id getNumber();

	/**
	* @NAME: getNumberUntil
	* @DESC: devuelve un numero entero positivo random de 0 hasta un rango maximo pasado por parametro
	*/
	id getNumberUntil(int maximo);

#endif /* RANDOM_RANDOM_NUMBERS_H_ */
