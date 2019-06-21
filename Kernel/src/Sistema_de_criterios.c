/*
 * Sistema_de_criterios.c
 *
 *  Created on: 20 jun. 2019
 *      Author: facusalerno
 */

#include "Sistema_de_criterios.h"

//FUNCIONES: Privadas
static void agregar_memoria_a_lista_criterios(Memoria *memoria, Consistencia consistencia);
static MetadataTabla *machearTabla(char *tabla);//Dado el nombre de una tabla devuelve la estructura que la representa. Si no existe devuelve null
static Memoria *machearMemoria(int numeroMemoria);
static Memoria *sc_determinar_memoria(MetadataTabla *tabla); //Dada una tabla, determina que memoria la deberia atender


Memoria *determinar_memoria_para_tabla(char *nombreTabla){ //No liberar nunca nombreTabla por que es: o parte de una linea lql o parte de una request individual. Lo que se tenga que liberar, se libera en Unidad_de_ejecucion.c
	MetadataTabla *tabla;
	if((tabla = machearTabla(nombreTabla)) == NULL)
		return NULL;

	switch(tabla->consistencia){
	case SC:
		return sc_determinar_memoria(tabla);
	case HSC:
		break;
	case EC:
		break;
	default:
		return NULL;
	}
	return NULL;
}





int add_memory(char *numeroMemoria, char *consistencia){
	Memoria *memoria;
	if((memoria = machearMemoria(atoi(numeroMemoria))) == NULL)
		RETURN_ERROR("Sistema_de_criterios.c: add_memory: el numero de memoria es invalido o aun no se conoce");

	if(string_equals_ignore_case(consistencia, "SC"))
		agregar_memoria_a_lista_criterios(memoria, SC);
	else if(string_equals_ignore_case(consistencia, "HSC"))
		agregar_memoria_a_lista_criterios(memoria, HSC);
		else if(string_equals_ignore_case(consistencia, "EC"))
			agregar_memoria_a_lista_criterios(memoria, EC);
		else RETURN_ERROR("Sistema_de_criterios.c: add_memory: el tipo de consistencia es invalido. Solo se admite SC, HSC o EC");
	return EXIT_SUCCESS;
}





static void agregar_memoria_a_lista_criterios(Memoria *memoria, Consistencia consistencia){
	switch(consistencia){
	case SC:
		list_add(memoriasSC, memoria);
		break;
	case HSC:
		list_add(memoriasSHC, memoria);
		break;
	case EC:
		list_add(memoriasEC, memoria);
		break;
	default:
		return;
	}
}




static MetadataTabla *machearTabla(char *tabla){
	if(tablasExistentes == NULL)
		return NULL;
	bool buscar(void *elemento){
		return !strcmp(tabla, ((MetadataTabla*)elemento)->nombre); //Devuelve 0 (falso) si son iguales
	}
	return (MetadataTabla*)list_find(tablasExistentes, buscar);
}





static Memoria *machearMemoria(int numeroMemoria){
	if(memoriasExistentes == NULL){
		return NULL;
	}
	bool buscar(void *elemento){
		return numeroMemoria == ((Memoria*)elemento)->numero;
	}
	return (Memoria*)list_find(memoriasExistentes, buscar);
}





static Memoria *sc_determinar_memoria(MetadataTabla *tabla){
	if(memoriasSC == NULL) //Si la lista es NULL significa que nunca se creo, por lo tanto asumo que trabajamos solo com memoria principal. Si la lista es vacia ese es otro tema
		return NULL;

	if(tabla->Consistencia.SC.memoriaAsignada == NULL){
		if(!list_is_empty(memoriasSC)){
			tabla->Consistencia.SC.memoriaAsignada = list_get(memoriasSC, getNumberUntil(list_size(memoriasSC)));
		}else{
			log_error(logger_error, "Sistema_de_criterios.c: sc_asociar_memoria_con_tabla: No se puede responder la request por que no hay memorias Strong Consistency disponibles");
			log_info(logger_invisible, "Sistema_de_criterios.c: sc_asociar_memoria_con_tabla: No se puede responder la request por que no hay memorias Strong Consistency disponibles");
			return NULL;
		}
	}
	return tabla->Consistencia.SC.memoriaAsignada;
}

