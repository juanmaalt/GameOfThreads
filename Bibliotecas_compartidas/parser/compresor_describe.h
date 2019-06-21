/*
 * compresor_describe.h
 *
 *  Created on: 20 jun. 2019
 *      Author: facusalerno
 */

#ifndef PARSER_COMPRESOR_DESCRIBE_H_
#define PARSER_COMPRESOR_DESCRIBE_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include "../consistencias/consistencias.h"

	/**
	* @NAME: concatenar_tabla
	* @DESC: concatena a la cadena source la secuencia tabla;consistencia. Una cadena procesada varias veces quedaria
	* 		 como tabla1;cons1;tabla2;cons2;tabla3;cons3 etc
	*/
	void concatenar_tabla(char **source, char *tabla, Consistencia consistencia);

	/**
	* @NAME: descomprimir_tablas
	* @DESC: descomprime una cadena en cad aparte separada por el ;
	*/
	char **descomprimir_tablas(char *compresion);


	/**
	* @NAME: destruir_split_tablas
	* @DESC: destruye cualquier operacion generada por los descompresores
	*/
	void destruir_split_tablas(char **split);



#endif /* PARSER_COMPRESOR_DESCRIBE_H_ */
