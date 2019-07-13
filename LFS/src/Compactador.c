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

	t_config* metadataFile = leerMetadata((char*)nombreTabla);
	getMetadata((char*)nombreTabla, metadataFile);

	for(;;){
		usleep(config_get_int_value(metadataFile, "COMPACTION_TIME") * 1000);

		log_info(logger_invisible, "Compactador.c: compactar() - Inicio compactación");
		//printf("path: %s\n", pathTabla);

		if((dir = opendir(pathTabla)) != NULL){
			/*Cambio el nombre de los archivos temporales de .tmp a .tmpc*/
			cambiarNombreFilesTemp(pathTabla);
			/*Agrego la tabla en el diccionario de compactación, para bloquear el acceso de las funciones que lleguen*/
			agregarTablaEnDiccCompactacion((char*)nombreTabla);
			/*Compacto los archivos .tmpc hasta que no haya más*/
			while((entry = readdir (dir)) != NULL){
				nombreArchivo = string_from_format(entry->d_name);
				if(string_contains(nombreArchivo, ".tmpc")){
					char* pathTemp = string_from_format("%s/%s", pathTabla, nombreArchivo);

					log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] es un archivo temporal, inciando su compactación.", (char*)nombreTabla, nombreArchivo);
					/*Leo el archivo temporal e inicio su compactación*/
					leerTemporal(pathTemp, metadata.partitions, (char*)nombreTabla);
					/*Borro el archivo temporal*/
					remove(pathTemp);
				}
				else{
					log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] no es un archivo temporal, no se compactará", (char*)nombreTabla, nombreArchivo);
				}
			}


		/*

			procesarPeticionesPendientes(diccCompactacion, nombreTabla); //Busca en el diccionario por el hash nombreTabla hace un pop de cada peticion y la manda a ejecutarOperacion
		 */
			sacarTablaDeDiccCompactacion((char*)nombreTabla);
			closedir (dir);
			log_info(logger_invisible, "Compactador.c: compactar() - Fin compactación");
		}
	}
	return NULL;
}

