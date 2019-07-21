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
		}SHC;
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
	* @NAME: elegir_cualquiera
	* @DESC: para aquellas situaciones donde no es importante el criterio. Por ejemplo, un describe general.
	* 		 se eligira por default una memoria EC
	*/
	Memoria *elegir_cualquiera();

	/**
	* @NAME: asociar_memoria
	* @DESC: asocia una memoria a un criterio, lo cual implica agregarla a la lista de memoriasSC, HSC o SC. Para eso
	* 		 se extrae una de la lista de memoriasExistentes. Esta ultima se rellena gracias al proceso de gossiping
	* 		 Se usa en la consola para dar servicio a la api ADD MEMORY TO CRITERIO
	*/
	int asociar_memoria(char *numeroMemoria, char *consistencia);


	/**
	* @NAME: procesar_describe_global
	* @DESC: una request describe devuelve como resultado una cadena con todas las metadatas de las tablas comprimidas en un
	* 		 char*. Asique esta funcion la descomprime y las va a agregando a la lista de metadatas de tablas, actualizando
	* 		 las viejas, removiendo las que ya no estan, y dando de alta las nuevas
	*/
	int procesar_describe_global(char *cadenaResultadoDescribe);

	/**
	* @NAME: procesar_describe_simple
	* @DESC: procesa el describe para una tabla en particular. La instruccion actual es necesaria para saber sobre que tabla es
	*/
	int procesar_describe_simple(char *cadenaResultadoDescribe, char *instruccionActual);


	/**
	* @NAME: procesar_gossiping
	* @DESC: procesa una cadena de gossiping y devuelve una lista de todas las memorias activas (levantadas)
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
	* @NAME: remover_metadata_tabla
	* @DESC: remueve de todos lados una tabla.
	*/
	void remover_metadata_tabla(MetadataTabla *tabla);

	/**
	* @NAME: agregar_sin_repetidos
	* @DESC: agrega los elementos de la lista fuente a la lista destino omitiendo los repetidos. Como se usa con memorias,
	* 		 se usa el numero de memoria para comparar. Ademas, remueve de la lista fuente el elemento que que pudo
	* 		 agregar, para que al final de la operacion se pueda hacer un destroy de la lista y sus nodos restantes.
	* 		 No es una funcion muy reutilizable si no le pasamos por argumento alguna closure de comparacion
	*/
	void agregar_sin_repetidos(t_list *destino, t_list *fuente);

	/**
	* @NAME: agregar_metadata_tabla
	* @DESC: agrega la metadata de una tabla a la lista de tablas existentes. Es util cuando se quiere agregar de forma
	* 		 manual ya sea por que se ejecuto un create o lo que fuera. De esta manera, se tiene la metadata de una tabla
	* 		 antes de hacer un describe (siempre y cuando el create se haga del kernel)
	*/
	void agregar_metadata_tabla(char *nombre, char *consistencia, char *particiones, char *tiempoEntreCompactacion);

	/**
	* @NAME: tabla_esta_en_la_lista
	* @DESC: dice si una tabla esta en la lista de tablas existentes. La busqueda es por nombre de tabla. O sea yo no pregunto
	* 		 si un determinado objeto tabla esta en la lista. Yo pregunto si el nombre de una tabla esta en la lista.
	*/
	bool tabla_esta_en_la_lista(char *tabla);

	/**
	* @NAME: eliminar_todas_las_tablas
	* @DESC: eliminar_todas_las_tablas de la lista de tablasExistentes borrando lista y nodos
	*/
	void eliminar_todas_las_tablas();

	/**
	* @NAME: eliminar_todas_las_memorias
	* @DESC: eliminar_todas_las_memorias de la lista pasada por arguento eliminando lista y nodos
	*/
	void eliminar_todas_las_memorias(t_list *lista);

#endif /* SISTEMA_DE_CRITERIOS_H_ */
