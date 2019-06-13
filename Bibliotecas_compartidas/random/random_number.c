/*
 * random_number.c
 *
 *  Created on: 13 jun. 2019
 *      Author: facusalerno
 */

#include "random_numbers.h"

id getNumber(){
	return rand() % (RANGO_MAX - RANGO_MIN +1) + RANGO_MIN;
}
