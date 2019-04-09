/*
 * parser.h
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 */

/*USO:
	SELECT nombreTabla Key
	INSERT nombreTabla key value timestamp (puede no tener timestamp)
	CREATE nombreTabla tipoConsistencia numeroParticiones compactacionTime
	DESCRIBE nombreTabla
	DROP nombreTabla
	JOURNAL
	ADD MEMORY numero TO criterio
	RUN path
Nota: no es a prueba de tontos
*/

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	char *palabraEspecial;
	char *nombreTabla;
	char *key;
	char *value;
	char *timestamp;
	char *tipoConsistencia;
	char *numeroParticiones;
	char *compactacionTime;
	char *numeroMemoria;
	char *criterio;
	char *path;
	int hayError; //1 si hay error en el parseo, 0 si no lo hay
	char *descripcionError;
} Comando;

//FUNCIONES
Comando *parsear_linea(char *);
void free_comando(Comando *comando);
char *obtener_palabra_especial(char *);
char *extraer_palabra_y_guardar(char *);
char *formatear_cadena(char *);


//VARIABLES GLOBALES
int seek; //Apuntador global al caracter de la linea que se esta parseando

#endif /* PARSER_H_ */
