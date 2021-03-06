/*
 * compresor_direccion.c
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */


#include "compresor_direccion.h"

void concatenar_memoria(char **source, char * numeroMemoria, char *ip, char *puerto){
	if(ip == NULL || puerto == NULL)
		return;
	if(*source == NULL)
		 *source = string_from_format("%s", numeroMemoria);
	else
		string_append(source, numeroMemoria/*string_from_format("%d", numeroMemoria)*/);
	string_append(source, ";");
	string_append(source, ip);
	string_append(source, ";");
	string_append(source, puerto);
	string_append(source, ";");

	free(numeroMemoria);
}

char **descomprimir_memoria(char *compresion){
	return string_split(compresion, ";");
}

void destruir_split_memorias(char **split){
	if(split){
		string_iterate_lines(split, (void *)free);
		free(split);
	}
}
