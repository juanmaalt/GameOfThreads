/*
 * Unidad_de_ejecucion.h
 *
 *  Created on: 8 may. 2019
 *      Author: utnso
 */

#ifndef UNIDAD_DE_EJECUCION_H_
#define UNIDAD_DE_EJECUCION_H_
#define TRUE 1
#define FALSE 0

	/*COMPORTAMIENTOS DE LA CPU*/
	/**
	* USAR_SOLO_MEMORIA_PRINCIPAL: Usa la memoria principal para atender todas las request sin importar si otras memorias
	* se sumen.
	*/
	#define USAR_SOLO_MEMORIA_PRINCIPAL FALSE

	/**
	* DELEGAR_A_MEMORIA_PRINCIPAL: Delega a la memoria principal todas las request que no hayan encontrado una memoria que las
	* ejecute. Por ejemplo si no hay memorias SC, entonces una request a una tabla SC seria ejecutada por la MP si esta
	* opcion esta activada.
	*/
	#define DELEGAR_A_MEMORIA_PRINCIPAL FALSE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/process.h>
#include <random/random_numbers.h>
#include "Planificador.h"
#include "Sistema_de_criterios.h"

//ESTRUCTURAS:
typedef struct pcb_t PCB;
typedef int socket_t;
typedef struct memoria Memoria;

//Internal states
#define INTERNAL_STATE int
#define CONTINUE -1
#define FINISH -2
#define INSTRUCCION_ERROR -3
#define DESALOJO -4
#define NULL_MEMORY -5
#define JOURNAL_REQUEST -6
#define JOURNAL_MEMORY_INACCESSIBLE -7
#define NULL_CONSISTENCY -8

typedef enum{
	LECTURA,
	ESCRITURA
}TipoOperacion;

typedef struct{
	socket_t socket;
	Memoria *memoria;
	Consistencia criterioQueSeUso;
	bool operacionExitosa;
	TipoOperacion tipoOperacion;
	timestamp_t inicioOperacion;
	timestamp_t finOperacion;
}DynamicAddressingRequest; //Como no es suficiente con saber el socket al cual quiero direccionar una request, recurro a esta estructura que contiene datos extras que se usan en las metricas. La idea es ir rellenandola a medida que se vayan coniciendo los datos y al final generar la metrica

//FUNCIONES: Publicas
void *exec(void *null);
int solicitar_socket(Memoria *memoria);

#endif /* UNIDAD_DE_EJECUCION_H_ */
