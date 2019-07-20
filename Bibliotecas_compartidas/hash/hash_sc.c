/*
 * hash_sc.c
 *
 *  Created on: 21 jun. 2019
 *      Author: facusalerno
 */

#include "hash_sc.h"

hash getHash(char *input, unsigned int limite){
	if(limite <= 1) //Si es 0, chau. Si es 1 significan dos cosas: Que hay una sola memoria por ende respondo con 0 que seria el primer elemento de la lista. Y ademas que cuando tenga que hacer el resto, me daria division por 0 por que hace limite-1
		return 0;
	int acumulador = 1;
	for(int i=0; i<strlen(input); ++i){
		acumulador += input[i];
	}
	return (acumulador % limite)-1;
}
