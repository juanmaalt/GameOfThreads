/*
 * hash_sc.h
 *
 *  Created on: 21 jun. 2019
 *      Author: facusalerno
 */

#ifndef HASH_HASH_SC_H_
#define HASH_HASH_SC_H_

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	typedef unsigned int hash;

	/**
	* @NAME: getHash
	* @DESC: devuelve un hash en base a una entrada, especificando un numero maximo. Esto significa que los valores
	* 		 posibles del hash estan comprendidos entre 0 y limite-1. Esto es util para que dado un nombre de una tabla,
	* 		 el hash devuelva alguna de las posibles memorias de la lista, siendo el limite la cantidad de elementos de la
	* 		 lista. En una lista de 5 elementos indexad desde 0 hasta 4, esta funcion da numeros comprendidos entre 0 y 4.
	*/
	hash getHash(char *input, unsigned int limite);

#endif /* HASH_HASH_SC_H_ */
