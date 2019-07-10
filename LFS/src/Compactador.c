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

				remove(pathTemp); //Descomentar cuando ande todo como se espera
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
    char value[atoi(config.tamanio_value)];

	temp= fopen(pathTemp, "r");
	while(fscanf(temp, "%d;%d;%[^\n]s", &timestamp, &key, value)!= EOF){
		printf("%d;%d;%s\n", timestamp, key ,value);
		char* linea = string_from_format("%d;%d;%s",timestamp, key, value);

		int particionNbr = calcularParticionNbr(string_from_format("%d", key), particiones);
		printf("Partición Nro: %d\n", particionNbr);

		char* listaDeBloques= obtenerListaDeBloques(particionNbr, nombreTabla);
		printf("bloques: %s\n",listaDeBloques);

		char* bloque = firstBloqueDisponible(listaDeBloques);
		printf("firstBloqueDisponible: %s\n", bloque);

		escribirEnBloque(bloque, linea);

		free(linea);
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

char* firstBloqueDisponible(char* listaDeBloques){
	char* pathBloques = malloc((strlen(config.punto_montaje)+250) * sizeof(char));
	strcpy(pathBloques,config.punto_montaje);
	strcat(pathBloques, "Bloques/");
	char* pathBloque = malloc((strlen(config.punto_montaje)+300) * sizeof(char));
	char** bloques = string_get_string_as_array(listaDeBloques);

	char* firstBloque=0;
	int i=0;

	while(bloques[i]!=NULL){
		int charsInFile = caracteresEnBloque(pathBloque,pathBloques,bloques[i]);
		if(charsInFile < metadataFS.blockSize){
			printf("Bloque %s con %d caracteres disponibles\n", bloques[i], (metadataFS.blockSize - charsInFile));
			free(pathBloque);
			free(pathBloques);
			firstBloque=bloques[i];
		}
		else{
			printf("Bloque %s sin caracteres disponibles\n", bloques[i]);
			firstBloque="0";
		}
		i++;
	}
	return firstBloque;
}

int caracteresEnBloque(char* pathBloque, char* pathBloques, char* bloque){
	strcpy(pathBloque,pathBloques);
	strcat(pathBloque, bloque);
	strcat(pathBloque, ".bin");
	//printf("path: %s\n", pathBloque);

	FILE* fBloque = fopen(pathBloque, "r+");
	char ch;
	int count=0;

	while((ch=fgetc(fBloque))!=EOF){
		count++;
	}
	//printf("la cantidad de caracteres en %s.bin es %d\n", bloque, count);

	fclose(fBloque);
	return count;
}


void escribirEnBloque(char* bloque, char* linea){
	char* pathBloque = malloc((strlen(config.punto_montaje)+250) * sizeof(char));
	strcpy(pathBloque,config.punto_montaje);
	strcat(pathBloque, "Bloques/");
	//char* pathBloqueNuevo = malloc((strlen(pathBloque)+250) * sizeof(char));

	if(string_equals_ignore_case(bloque, "0")){
		bloque = string_from_format("%d",getBloqueLibre());
	}
	strcat(pathBloque, bloque);
	strcat(pathBloque, ".bin");

	printf("pathBloque: %s\n", pathBloque);

	FILE* fBloque = fopen(pathBloque, "a");

	fprintf (fBloque, "%s",linea);

	printf("linea:%s\n", linea);

	fclose(fBloque);
	free(pathBloque);
}
