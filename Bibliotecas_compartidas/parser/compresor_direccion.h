/*
 * compresor_direccion.h
 *
 *  Created on: 10 jun. 2019
 *      Author: facusalerno
 */

#ifndef PARSER_COMPRESOR_DIRECCION_H_
#define PARSER_COMPRESOR_DIRECCION_H_

	#include <stdlib.h>
	#include <stdio.h>
	#include <commons/string.h>
	#include <string.h>

	/**
	* @NAME: concatenar_tabla
	* @DESC: concatena a la cadena source la secuencia nro;ip;puerto. Una cadena procesada varias veces quedaria
	* 		 como nro1;ip1;puerto1;nro2;ip2;puerto2 etc
	*/
	void concatenar_memoria(char **source, int  numeroMemoria, char *ip, char *puerto);

	/**
	* @NAME: descomprimir_memoria
	* @DESC: descomprime una cadena comprimida en cada partes separada por el ;
	*/
	char **descomprimir_memoria(char *compresion);


	/**
	* @NAME: destruir_split_memorias
	* @DESC: destruye cualquier operacion generada por los descompresores
	*/
	void destruir_split_memorias(char **split);

#endif /* PARSER_COMPRESOR_DIRECCION_H_ */
