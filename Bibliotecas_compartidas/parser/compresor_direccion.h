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
	* @NAME: comprimir_direcion
	* @DESC: comprime una direccion ip y un puerto al formato ip:puerto
	* @PARAMS:
	* 		ip - puerto
	*/
	char *comprimir_direccion(const char* ip, const char* puerto);


	/**
	* @NAME: descomprimir_direccion
	* @DESC: devuelve un array de strings para acceder individualmente a la ip y puerto.
	* 		 por ejemplo char** direcc = descomprimir_direccion("127.0.0.0:8080"); va a ser accesible
	* 		 como direcc[0] y direcc[1]
	* @PARAMS:
	* 		ip - puerto
	*/
	char **descomprimir_direccion(char*direccion);


	/**
	* @NAME: descomprimir_direccion
	* @DESC: libera el arrays de strings generado por descomprimir direccion
	* @PARAMS:
	* 		split - operacion generada por descomprimir_direccion
	*/
	void destruir_direcciones(char **split);

#endif /* PARSER_COMPRESOR_DIRECCION_H_ */
