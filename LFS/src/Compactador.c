/*
 * Compactador.c
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#include "Compactador.h"

void compactar(char* nombreTabla){
	char* pathTabla = malloc(1000 * sizeof(char));
	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	strcpy(pathTabla,config.punto_montaje);
	strcat(pathTabla, "Tables/");
	strcat(pathTabla, nombreTabla);

	t_config* metadataFile = leerMetadata(nombreTabla);
	if(getMetadata(nombreTabla, metadataFile)==EXIT_FAILURE){
		printf("No existe el archivo Metadata de la tabla solicitada.\n");
	}

	//printf("path: %s\n", pathTabla);

	if((dir = opendir(pathTabla)) != NULL){
		/*Cambio el nombre de los archivos temporales de .tmp a .tmpc*/
		cambiarNombreFilesTemp(pathTabla);
		/*Agrego la tabla en el diccionario de compactación, para bloquear el acceso de las funciones que lleguen*/
		agregarTablaEnDiccCompactacion(nombreTabla);

		/*Compacto los archivos .tmpc hasta que no haya más*/
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = string_from_format(entry->d_name);
			if(string_contains(nombreArchivo, ".tmpc")){
				char* pathTemp = malloc(1000 * sizeof(char));

				strcpy(pathTemp, pathTabla);
				strcat(pathTemp, "/");
				strcat(pathTemp, nombreArchivo);

			    log_info(logger_visible, "Compactar(%s): [%s] es un archivo temporal, compactar\n", nombreTabla, nombreArchivo);

			    leerTemporal(pathTemp, metadata.partitions, nombreTabla);

				//remove(pathFile); //Descomentar cuando ande todo como se espera
			}
			else{
				log_info(logger_invisible, "Compactar(%s): [%s] no es un archivo temporal, no compactar\n", nombreTabla, nombreArchivo);
			}
		}


	/*
	while(hayaFileTemps){
		FILE* temp;

		temp = abrirArchivoTemp();

		while(leer(temp)!=EOF){
			leerLinea();
			calcularParticion();
			agregarBloqueEnParticion();
			escribirLineaEnBloque(); //Ver el primer bloque disponible, ver cuanto lenght queda, escribir hasta donde se pueda y lo que sigue en otro bloque
		}
	}

	procesarPeticionesPendientes(diccCompactacion, nombreTabla); //Busca en el diccionario por el hash nombreTabla hace un pop de cada peticion y la manda a ejecutarOperacion
*/
		sacarTablaDeDiccCompactacion(nombreTabla);
		closedir (dir);
	}
	free(pathTabla);
}


void leerTemporal(char* pathTemp, int particiones, char* nombreTabla){
	FILE* temp;
    int timestamp, key;
    char value[1000];

	temp= fopen(pathTemp, "r");
	while(fscanf(temp, "%d;%d;%[^\n]s", &timestamp, &key, value)!= EOF){
		printf("%d;%d;%s\n", timestamp, key ,value);
		int particionNbr = calcularParticionNbr(string_from_format("%d", key), particiones);
		printf("Partición Nro: %d\n", particionNbr);

		char* listaDeBloques= obtenerListaDeBloques(particionNbr, nombreTabla);
		printf("bloques: %s\n",listaDeBloques);

		readAndWriteBloque(listaDeBloques);

	}
	fclose(temp);
}

char* obtenerListaDeBloques(int particion, char* nombreTabla){
	char* pathFile = malloc(1000 * sizeof(char));
	strcpy(pathFile,config.punto_montaje);
	strcat(pathFile, "Tables/");
	strcat(pathFile, nombreTabla);
	strcat(pathFile, "/");
	strcat(pathFile, string_from_format("%d", particion));
	strcat(pathFile, ".bin");

	t_config* particionFile;
	particionFile = config_create(pathFile);
	char* resultado = config_get_string_value(particionFile, "BLOCKS");
	char* listaDeBloques = string_from_format(resultado);

	config_destroy(particionFile);
	free(pathFile);

	return listaDeBloques;
}

void readAndWriteBloque(char* listaDeBloques){
	char* pathBloques = malloc(1000 * sizeof(char));
	strcpy(pathBloques,config.punto_montaje);
	strcat(pathBloques, "Bloques/");
	char* pathBloque = malloc(1000 * sizeof(char));

	char** bloques = string_get_string_as_array(listaDeBloques);

	for(int i=0;bloques[i]!=NULL;i++){
		strcpy(pathBloque,pathBloques);
		strcat(pathBloque, bloques[i]);
		strcat(pathBloque, ".bin");
		//printf("path: %s\n", pathBloque);

		FILE* fBloque = fopen(pathBloque, "r+");
		char ch;
		int count=0;

		while((ch=fgetc(fBloque))!=EOF){
			count++;
		}
		printf("la cantidad de caracteres en %s.bin es %d\n", bloques[i], count);

		fclose(fBloque);
	}

	free(pathBloque);
	free(pathBloques);
}





