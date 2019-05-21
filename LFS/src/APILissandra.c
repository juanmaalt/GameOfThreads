/*
 * APILissandra.c
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#include "APILissandra.h"



/*INICIO FUNCIONES API*/

int selectAPI(Comando comando){
	if (!memtable) {
		log_error(logger_invisible, "No existe una memtable creada, no se puede realizar la operación.");
		return EXIT_FAILURE;
	}

	if(!(existeTabla(comando.argumentos.SELECT.nombreTabla))){
		log_error(logger_invisible, "No existe una tabla asociada a la key solicitada.");
		//avisar a la memoria?
		return EXIT_FAILURE;
	}

	t_list* data = list_create();
	data = getData(comando.argumentos.SELECT.nombreTabla);

	Metadata_tabla* metadata = malloc(sizeof(Metadata_tabla));
	metadata = getMetadataValues(data);

	int particionNbr = calcularParticionNbr(comando.argumentos.SELECT.key, metadata->partitions);

	t_list* listaDeValues = buscarValue(data, comando.argumentos.SELECT.key, particionNbr);

	/*
		getMasReciente([KEY]); //Encontradas las entradas para dicha Key, se retorna el valor con el Timestamp más grande.
	*/

	list_destroy(data);
	list_destroy(listaDeValues);

	return EXIT_SUCCESS;
}


void insertAPI(Comando comando){
	/*

	*/
}


void createAPI(Comando comando){
}

void describeAPI(Comando comando){
}

void dropAPI(Comando comando){
}
/*FIN FUNCIONES API*/


/*INICIO FUNCIONES COMPLEMENTARIAS*/
bool existeTabla(char* nombreTabla){
	return dictionary_has_key(memtable, nombreTabla);
}

t_list*	getData(char* nombreTabla){
	return dictionary_get(memtable, nombreTabla);
}

Metadata_tabla* getMetadataValues(t_list* data){
	return list_get(data,0);
}

int calcularParticionNbr(char* key, int particiones){
	return atoi(key)/particiones;
}

t_list* buscarValue(t_list* data, char* key, int particionNbr){
	printf("Numero de partición: %d", particionNbr);

	bool compararConItem(void* item){
		if (atoi(key) != obtenerKey((registro*) item)) {
			return false;
		}
		return true;
	}

	return list_filter(data, compararConItem);
}


/*FIN FUNCIONES COMPLEMENTARIAS*/

