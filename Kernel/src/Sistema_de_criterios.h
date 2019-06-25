/*
 * Sistema_de_criterios.h
 *
 *  Created on: 20 jun. 2019
 *      Author: facusalerno
 */

#ifndef SISTEMA_DE_CRITERIOS_H_
#define SISTEMA_DE_CRITERIOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <consistencias/consistencias.h>
#include <commons/collections/list.h>
#include <random/random_numbers.h>
#include <hash/hash_sc.h>
#include <parser/compresor_describe.h>
#include <parser/compresor_direccion.h>
#include <stdbool.h>
#include "Unidad_de_ejecucion.h"

//ESTRUCTURAS
typedef struct {
	int numero;
	char *ip;
	char *puerto;
}Memoria; //Toda memoria conocida va a tener asignada alguna categoria lo cual significa que va a estar dentro de su respectiva lista. La memoria principal no es la excepcion. Aunque a esa la puedo conocer por config, no se le puede asignar ninguna request



typedef struct {
	char *nombre;
	Consistencia consistencia;
	int particiones;
	int tiempoEntreCompactaciones;
	union{
		struct{
			Memoria *memoriaAsignada;
		}SC;
		struct{
		}SHC;
		struct{
		}EC;
	}Atributos;
}MetadataTabla;



//VARIABLES: Globales
t_list *memoriasSC;
t_list *memoriasHSC;
t_list *memoriasEC;
t_list *memoriasExistentes; //Union de los 3 conjuntos de arriba
t_list *tablasExistentes;



//FUNCIONES: Publicas
int iniciar_sistema_de_criterios(void);
Memoria *determinar_memoria_para_tabla(char *tabla);
int add_memory(char *numeroMemoria, char *consistencia); //Asocia una memoria a una consistencia para poder usarla. Internamente chequea los valores de consistencia y numero para que no se le puedan enviar cosas chanchas
int procesar_describe(char *cadenaResultadoDescribe);
int procesar_gossiping(char *cadenaResultadoGossiping);
void mostrar_describe(char *cadenaResultadoDescribe);

#endif /* SISTEMA_DE_CRITERIOS_H_ */
