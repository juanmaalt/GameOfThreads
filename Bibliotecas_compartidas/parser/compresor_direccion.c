/*
 * compresor_direccion.c
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */


#include "compresor_direccion.h"

char *comprimir_direccion(const char* ip, const char* puerto){
	return string_from_format("%s:%s", ip, puerto);
}

char **descomprimir_direccion(char *direccion){
	return string_split(direccion, ":");
}

void destruir_direcciones(char **split){
	if(split){
		string_iterate_lines(split, (void *)free);
		free(split);
	}
}
