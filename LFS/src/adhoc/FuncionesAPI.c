/*
 * FuncionesAPI.c
 *
 *  Created on: 23 jun. 2019
 *      Author: juanmaalt
 */

#include "FuncionesAPI.h"

/*INICIO FUNCIONES COMPLEMENTARIAS*/
bool existeTabla(char* nombreTabla){
	return dictionary_has_key(memtable, nombreTabla);

	//TODO:Checkear en disco, no en la memtable?
}


int getMetadata(char* nombreTabla, t_config* metadataFile){
	if(metadataFile == NULL){
		printf(RED"APILissandra.c: leerMetadata: error en el archivo 'Metadata' de la tabla %s"STD"\n", nombreTabla);
		return EXIT_FAILURE;
	}

	extraerMetadata(metadataFile);

	//mostrarMetadata();//funcion adhoc para testing

	return EXIT_SUCCESS;
}


t_config* leerMetadata(char* nombreTabla){
	char* path = malloc(100 * sizeof(char));

	strcpy(path,config.punto_montaje);
	strcat(path, "Tables/");
	strcat(path, nombreTabla);
	strcat(path, "/");
	strcat(path, "Metadata");

	return config_create(path);
}


void extraerMetadata(t_config* metadataFile) {
	metadata.compaction_time = config_get_int_value(metadataFile, "COMPACTION_TIME");
	metadata.consistency = config_get_string_value(metadataFile, "CONSISTENCY");
	metadata.partitions= config_get_int_value(metadataFile, "PARTITIONS");
}


void mostrarMetadata(){
	log_info(logger_visible, "\nMetadata->compaction_time= %d\n", metadata.compaction_time);
	log_info(logger_visible, "Metadata->consistency= %s\n", metadata.consistency);
	log_info(logger_visible, "Metadata->partitions= %d\n", metadata.partitions);
}


t_list*	getData(char* nombreTabla){
	return dictionary_get(memtable, nombreTabla);
}


int calcularParticionNbr(char* key, int particiones){
	return atoi(key)%particiones;
}


t_list* buscarValueEnLista(t_list* data, char* key){
	//printf("Numero de partición: %d\n\n", particionNbr);

	bool compararConItem(void* item){
		if (atoi(key) != obtenerKey((Registro*) item)) {
			return false;
		}
		return true;
	}

	return list_filter(data, compararConItem);
}

void buscarValue(t_list* data, t_list* listaDeValues, char* key, int particionNbr){
	listaDeValues=buscarValueEnLista(data, key);


}

void recorrerTabla(t_list* lista){
	Registro* reg= NULL;
	int i=0;

	while(!(list_get(lista, i)==NULL)){
		reg = list_get(lista, i);

		printf("Registro->Timestamp= %llu\n", reg->timestamp);
		printf("Registro->Key= %d\n", reg->key);
		printf("Registro->Value= %s\n", reg->value);

		i++;
	}
}


Operacion getValueMasReciente(t_list* lista){
	Operacion op;

	if(list_size(lista)>0){
			bool compararFechas(void* item1, void* item2){
			if (obtenerTimestamp((Registro*) item1) < obtenerTimestamp((Registro*) item2)) {
				return false;
			}
			return true;
		}

		list_sort(lista, compararFechas);

		Registro* reg = list_get(lista, 0);

		op.TipoDeMensaje = REGISTRO;
		op.Argumentos.REGISTRO.timestamp=reg->timestamp;
		op.Argumentos.REGISTRO.key=reg->key;
		op.Argumentos.REGISTRO.value=string_from_format(reg->value);

	}else{
		op.TipoDeMensaje = ERROR;
		op.Argumentos.ERROR.mensajeError = string_from_format("No existen registros relacionados con la key solicitada");
	}


	return op;
}


timestamp_t checkTimestamp(char* timestamp){
	if(timestamp==NULL){
		return getCurrentTime();
	}
	else{
		//printf("time= %llu\n",atoll(timestamp));
		return atoll(timestamp);
	}
}


void crearTablaEnMemtable(char* nombreTabla){
	t_list* lista = list_create();
	char* tabla=nombreTabla;

	dictionary_put(memtable, tabla, lista);
}


void setPathTabla(char* path, char* nombreTabla){
	strcpy(path,config.punto_montaje);
	strcat(path, "Tables/");
	strcat(path, nombreTabla);
}

void crearDirectorioTabla(char* path){
	crearDirectorio(path);
	strcat(path,"/");
}


void crearArchivo(char* path, char* nombre){
	char* pathArchivo = malloc(110 * sizeof(char));
	strcpy(pathArchivo,path);
	strcat(pathArchivo, nombre);

	//printf("archivo creado en: %s\n", pathArchivo);

	FILE* file=NULL;
	file = fopen(pathArchivo,"w");

	fclose(file);
	free(pathArchivo);
}

void escribirArchivoMetadata(char* path, Comando comando){
	FILE* fmetadata;

	char* pathArchivo = malloc(110 * sizeof(char));

	strcpy(pathArchivo,path);
	strcat(pathArchivo, "Metadata");

	fmetadata = fopen(pathArchivo,"a");

	fprintf (fmetadata, "COMPACTION_TIME=%d\n",atoi(comando.argumentos.CREATE.compactacionTime));
	fprintf (fmetadata, "CONSISTENCY=%s\n",comando.argumentos.CREATE.tipoConsistencia);
	fprintf (fmetadata, "PARTITIONS=%d\n",atoi(comando.argumentos.CREATE.numeroParticiones));

	fclose(fmetadata);
	free(pathArchivo);
}

void crearArchivosBinarios(char* path, int particiones){
	char filename[8];
	for(int i=0;i<particiones;i++){
		sprintf(filename, "%d.bin", i);
		crearArchivo(path, filename);
	}
}

void insertInFile(char* path, int particionNbr, char* key, char* value){
	FILE* fParticion;

	char* pathArchivo = malloc(2000 * sizeof(char));
	char filename[6];
	sprintf(filename, "%d.bin", particionNbr);

	strcpy(pathArchivo,path);
	strcat(pathArchivo, "/");
	strcat(pathArchivo, filename);

	fParticion = fopen(pathArchivo,"a");

	char* keyValue = malloc(1000 * sizeof(char));
	strcpy(keyValue, key);
	strcat(keyValue, ";");
	strcat(keyValue, value);



	fprintf (fParticion, "%s",keyValue);

	free(keyValue);
	fclose(fParticion);
	free(pathArchivo);

}

void getStringDescribe(char* path, char* pathMetadata, char* string, char* nombreTabla, Operacion *resultadoDescribe){
	DIR *dir;
	struct dirent *entry;
	char* nombreCarpeta;
	t_config* metadata;

	if(nombreTabla==NULL){
		if((dir = opendir(path)) != NULL){
			while((entry = readdir (dir)) != NULL){
				nombreCarpeta = string_from_format(entry->d_name);
				if(!strcmp(nombreCarpeta, ".") || !strcmp(nombreCarpeta, "..")){
				}else{
					strcat(pathMetadata, nombreCarpeta);
					strcat(pathMetadata, "/Metadata");
					//printf("path: %s\n", pathMetadata);
					//printf("nombreTabla: %s\n", nombreCarpeta);

					metadata = config_create(pathMetadata);
					char* consistencia = config_get_string_value(metadata, "CONSISTENCY");
					int compactionTime=config_get_int_value(metadata, "COMPACTION_TIME");
					int particiones =config_get_int_value(metadata, "PARTITIONS");

					concatenar_tabla(&string, nombreCarpeta, consistencia, particiones, compactionTime);
					//printf("string: %s\n", string);

					strcpy(pathMetadata,path);
				}
		  }
			closedir (dir);
			resultadoDescribe->TipoDeMensaje= DESCRIBE_REQUEST;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = string_from_format(string);
		}else{
			resultadoDescribe->Argumentos.ERROR.mensajeError = string_from_format("No hay carpetas creadas en el sistema");
		}
	}else{
		strcat(pathMetadata, nombreTabla);
		if((dir = opendir (path)) != NULL){
			strcat(pathMetadata, "/Metadata");
			//printf("path: %s\n", pathMetadata);
			//printf("nombreTabla: %s\n", nombreTabla);

			metadata = config_create(pathMetadata);
			char* consistencia = config_get_string_value(metadata, "CONSISTENCY");
			int compactionTime=config_get_int_value(metadata, "COMPACTION_TIME");
			int particiones =config_get_int_value(metadata, "PARTITIONS");

			concatenar_tabla(&string, nombreTabla, consistencia, particiones, compactionTime);
			//printf("string: %s\n", string);
			resultadoDescribe->TipoDeMensaje= DESCRIBE_REQUEST;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = string_from_format(string);
		}
		else{
			resultadoDescribe->Argumentos.ERROR.mensajeError = string_from_format("No existe la carpeta solicitada");
		}
	}

	config_destroy(metadata);

	//printf("string final: %s\n", string);
}

/*FIN FUNCIONES COMPLEMENTARIAS*/
