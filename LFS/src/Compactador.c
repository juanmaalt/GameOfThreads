/*
 * Compactador.c
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#include "Compactador.h"

void* compactar(void* nombreTabla){
	pthread_detach(pthread_self());

	char* pathTabla = string_from_format("%sTables/%s", config.punto_montaje, (char*)nombreTabla);
	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	//printf("path: %s\n", pathTabla);

	t_config* metadataFile = leerMetadata(nombreTabla);
	if(metadataFile == NULL){
		log_error(logger_visible, "Compactador.c: compactar(%s) - Error en el archivo 'Metadata' de la tabla %s, la tablara no se compactará.", (char*)nombreTabla, (char*)nombreTabla);
		log_error(logger_error, "Compactador.c: compactar(%s) - Error en el archivo 'Metadata' de la tabla %s, la tablara no se compactará.", (char*)nombreTabla, (char*)nombreTabla);
		return 0;
	}

	Metadata_tabla metadata;
	metadata.compaction_time = config_get_int_value(metadataFile, "COMPACTION_TIME");
	metadata.consistency = config_get_string_value(metadataFile, "CONSISTENCY");
	metadata.partitions= config_get_int_value(metadataFile, "PARTITIONS");

	//printf("Metadata->compaction_time= %d\n", metadata.compaction_time);
	//printf("Metadata->consistency= %s\n", metadata.consistency);
	//printf("Metadata->partitions= %d\n", metadata.partitions);

	for(;;){
		usleep(metadata.compaction_time * 1000);

		log_info(logger_invisible, "Compactador.c: compactar() - Inicio compactación");
		//printf("path: %s\n", pathTabla);

		if((dir = opendir(pathTabla)) != NULL){
			/*Cambio el nombre de los archivos temporales de .tmp a .tmpc*/
			cambiarNombreFilesTemp(pathTabla);
			/*Agrego la tabla en el diccionario de compactación, para bloquear el acceso de las funciones que lleguen*/
			//agregarTablaEnDiccCompactacion((char*)nombreTabla);
			/*Compacto los archivos .tmpc hasta que no haya más*/
			while((entry = readdir (dir)) != NULL){
				nombreArchivo = string_from_format(entry->d_name);
				if(string_contains(nombreArchivo, ".tmpc")){
					char* pathTemp = string_from_format("%s/%s", pathTabla, nombreArchivo);

					log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] es un archivo temporal, inciando su compactacion.", (char*)nombreTabla, nombreArchivo);
					/*Leo el archivo temporal e inicio su compactación*/
						//printf("llega a leer temporal\n");
						//usleep(5000000);
					leerTemporal(pathTemp, metadata.partitions, (char*)nombreTabla);
						//printf("pasó leer temporal\n");
					/*Borro el archivo temporal*/
					remove(pathTemp);
						//printf("pasó remover temp\n");
					free(pathTemp);
				}
				else{
					log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] no es un archivo temporal, no se compactara", (char*)nombreTabla, nombreArchivo);
				}
				free(nombreArchivo);
			}
			/*Busca en el diccionario por el hash nombreTabla hace un pop de cada peticion y la manda a ejecutarOperacion*/
			/*
			procesarPeticionesPendientes((char*)nombreTabla);

			sacarTablaDeDiccCompactacion((char*)nombreTabla);
			log_info(logger_invisible, "Compactador.c: compactar(%s) - Fin compactación", (char*)nombreTabla);
			*/
			closedir (dir);
		}
	}
	config_destroy(metadataFile);
	free(pathTabla);
	return NULL;
}

