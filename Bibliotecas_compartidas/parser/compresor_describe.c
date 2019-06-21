/*
 * compresor_describe.c
 *
 *  Created on: 20 jun. 2019
 *      Author: facusalerno
 */

#include "compresor_describe.h"

void concatenar_tabla(char **source, char *tabla, Consistencia consistencia){
	string_append(source, tabla);
	string_append(source, ";");
	string_append(source, string_from_format("%d", consistencia));
	string_append(source, ";");
}



char **descomprimir_describe(char *compresion){
	return string_split(compresion, ";");
}



void destruir_split(char **split){
	if(split){
		string_iterate_lines(split, (void *)free);
		free(split);
	}
}

