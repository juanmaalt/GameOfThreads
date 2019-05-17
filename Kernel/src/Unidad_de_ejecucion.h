/*
 * Unidad_de_ejecucion.h
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#ifndef UNIDAD_DE_EJECUCION_H_
#define UNIDAD_DE_EJECUCION_H_
#define RED "\x1b[31m"
#define STD "\x1b[0m"

#include <stdio.h>
#include <stdlib.h>
#include "Planificador.h"

//FUNCIONES
void *exec(void *);
int exec_file_lql(PCB *pcb);
int exec_string_comando(PCB *pcb);
Comando *decodificar_siguiente_instruccion(FILE *lql);

#endif /* UNIDAD_DE_EJECUCION_H_ */
