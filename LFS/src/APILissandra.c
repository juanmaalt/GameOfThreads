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

	t_list* data = getData(comando.argumentos.SELECT.nombreTabla);

	Metadata_tabla* metadata = getMetadataValues(data);

	mostrarMetadata(metadata);//funcion adhoc para testing

	int particionNbr = calcularParticionNbr(comando.argumentos.SELECT.key, metadata->partitions);

	t_list* listaDeValues = list_create();
	listaDeValues = buscarValue(data, comando.argumentos.SELECT.key, particionNbr);

	recorrerTabla(listaDeValues);

	getValueMasReciente(listaDeValues);


	//list_destroy(data);
	list_destroy(listaDeValues);

	return EXIT_SUCCESS;
}


int insertAPI(Comando comando){
	if (!memtable) {
		log_error(logger_invisible, "No existe una memtable creada, no se puede realizar la operación.");
		return EXIT_FAILURE;
	}

	if(!(existeTabla(comando.argumentos.SELECT.nombreTabla))){
		log_error(logger_invisible, "No existe una tabla asociada a la key solicitada.");
		//avisar a la memoria?
		return EXIT_FAILURE;
	}

	//checkExisteMemoria(); //Verificar si existe en memoria una lista de datos a dumpear. De no existir, alocar dicha memoria.

	printf("antes del strcmp\n");


	Registro* reg = malloc(sizeof(Registro));
	reg->key = atoi(comando.argumentos.INSERT.key);
	reg->value = comando.argumentos.INSERT.value;
	reg->timestamp=checkTimestamp(comando.argumentos.INSERT.timestamp);

	t_list* data = getData(comando.argumentos.INSERT.nombreTabla);
	list_add(data, reg);

	printf("Registro1->Timestamp= %llu\n", reg->timestamp);
	printf("Registro1->Key= %d\n", reg->key);
	printf("Registro1->Value= %s\n", reg->value);




	return EXIT_SUCCESS;
}


int createAPI(Comando comando){
	if(!(existeTabla(comando.argumentos.CREATE.nombreTabla))){
		log_error(logger_invisible, "No existe una tabla asociada a la key solicitada.");
		//avisar a la memoria?
		return EXIT_FAILURE;
	}

	//crearDirectorio(); //Crear el directorio para dicha tabla.
	//crearMetadata(); //Crear el archivo Metadata asociado al mismo.
	//escribirMetadata(); //Grabar en dicho archivo los parámetros pasados por el request.
	//crearArchivosBinarios(); //Crear los archivos binarios asociados a cada partición de la tabla y asignar a cada uno un bloque

	Metadata_tabla* meta = malloc(sizeof(Metadata_tabla));

	meta->compaction_time=atoi(comando.argumentos.CREATE.compactacionTime);
	meta->consistency=comando.argumentos.CREATE.tipoConsistencia;
	meta->partitions=atoi(comando.argumentos.CREATE.numeroParticiones);

	crearTablaEnMemtable(comando.argumentos.CREATE.nombreTabla);

	return EXIT_SUCCESS;
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
	return atoi(key)%particiones;
}

t_list* buscarValue(t_list* data, char* key, int particionNbr){
	printf("Numero de partición: %d\n", particionNbr);

	bool compararConItem(void* item){
		if (atoi(key) != obtenerKey((Registro*) item)) {
			return false;
		}
		return true;
	}

	return list_filter(data, compararConItem);
}

void mostrarMetadata(Metadata_tabla* metadata){
	printf("\nMetadata->compaction_time= %d\n", metadata->compaction_time);
	printf("Metadata->consistency= %s\n", metadata->consistency);
	printf("Metadata->partitions= %d\n", metadata->partitions);

}

void recorrerTabla(t_list* lista){
	Registro* reg= NULL;
	int i=0;

	while(!(list_get(lista, i)==NULL)){
		reg = list_get(lista, i);

		printf("Registro1->Timestamp= %llu\n", reg->timestamp);
		printf("Registro1->Key= %d\n", reg->key);
		printf("Registro1->Value= %s\n", reg->value);

		i++;
	}
}

void getValueMasReciente(t_list* lista){

	bool compararFechas(void* item1, void* item2){
		if (obtenerTimestamp((Registro*) item1) < obtenerTimestamp((Registro*) item2)) {
			return false;
		}
		return true;
	}

	list_sort(lista, compararFechas);


	Registro* reg = malloc(sizeof(Registro));
	reg = list_get(lista, 0);

	printf("\nEl registro más reciente es:\n");
	printf("Registro->Timestamp= %llu\n",reg->timestamp );
	printf("Registro->Key= %d\n", reg->key);
	printf("Registro->Value= %s\n", reg->value);

	free(reg);

}

timestamp_t checkTimestamp(char* timestamp){
	if(timestamp==NULL){
		return getCurrentTime();
	}
	else{
		printf("time= %llu\n",atoll(timestamp));
		return atoll(timestamp);
	}
}

void crearTablaEnMemtable(char* nombreTabla){
	t_list* lista = list_create();
	char* tabla=nombreTabla;

	dictionary_put(memtable, tabla, lista);
}

/*FIN FUNCIONES COMPLEMENTARIAS*/

