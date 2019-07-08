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
typedef struct memoria{
	int numero;
	char *ip;
	char *puerto;
	bool fueAsociada; //A algun criterio cualquiera como minimo
	struct{
		struct{
			bool estaAsociada; //Para hacer metricas mas limpias
			int cantidadInsert;
			int cantidadSelect;
		}SC;
		struct{
			bool estaAsociada;
			int cantidadInsert;
			int cantidadSelect;
		}HSC;
		struct{
			bool estaAsociada;
			int cantidadInsert;
			int cantidadSelect;
		}EC;
	}Metrics;
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
		}HSC;
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

	/**
	* @NAME: iniciar_sistema_de_criterios
	* @DESC: inicializa todas las listas y estructuras para poder usar el sistema de criterios. Devuelve
	* 		 codigo de error (EXIT_FAILURE O EXIT_SUCCESS)
	*/
	int iniciar_sistema_de_criterios(void);


	/**
	* @NAME: determinar_memoria_para_tabla
	* @DESC: dado el nombre de una tabla, hace lo necesario para devolver una memoria sobre la cual ejecutar
	* 		 la request. En el caso de que esa tabla sea HSC, usa la key para averiguar a que memoria debe dirigir
	* 		 la request. Devuelve NULL si no encontro memoria. Por ejemplo, una request hacia la tabla ASD implica
	* 		 buscar en la lista de metadatas de tabla, para ver que consistencia tiene etc etc y devolver una memoria
	*/
	Memoria *determinar_memoria_para_tabla(char *tabla, char *keyDeSerNecesaria);


	/**
	* @NAME: asociar_memoria
	* @DESC: asocia una memoria a un criterio, lo cual implica agregarla a la lista de memoriasSC, HSC o SC. Para eso
	* 		 se extrae una de la lista de memoriasExistentes. Esta ultima se rellena gracias al proceso de gossiping
	* 		 Se usa en la consola para dar servicio a la api ADD MEMORY TO CRITERIO
	*/
	int asociar_memoria(char *numeroMemoria, char *consistencia);


	/**
	* @NAME: procesar_describe
	* @DESC: una request describe devuelve como resultado una cadena con todas las metadatas de las tablas comprimidas en un
	* 		 char*. Asique esta funcion la descomprime y las va a agregando a la lista de metadatas de tablas, actualizando
	* 		 las viejas, removiendo las que ya no estan, y dando de alta las nuevas
	*/
	int procesar_describe(char *cadenaResultadoDescribe);


	/**
	* @NAME: procesar_gossiping
	* @DESC: TODO
	*/
	t_list *procesar_gossiping(char *cadenaResultadoGossiping);


	/**
	* @NAME: mostrar_describe
	* @DESC: muestra el describe por pantalla. Tambien puede mostrarse el describe accediento a la lista de tablasExistentes
	* 		 Se puede usar para debugear
	*/
	void mostrar_describe(char *cadenaResultadoDescribe);

	/**
	* @NAME: consistencia_de_tabla
	* @DESC: devuelve el enum de la consistencia de una tabla, -1 si no encontro nada
	*/
	Consistencia consistencia_de_tabla(char *nombreTabla);

	/**
	* @NAME: remover_memoria
	* @DESC: remueve de todos lados una memoria. Por ejemplo si nos intentamos conectar y su socket parece offline,
	*        podemos removerla usando esta funcion
	*/
	void remover_memoria(Memoria *memoria);

	/**
	* @NAME: agregar_sin_repetidos
	* @DESC: TODO
	*/
	void agregar_sin_repetidos(t_list *destino, t_list *fuente);

#endif /* SISTEMA_DE_CRITERIOS_H_ */
