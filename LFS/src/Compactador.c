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
	strcat(pathTabla, "Tables");
	strcat(pathTabla, "/");
	strcat(pathTabla, nombreTabla);

	printf("path: %s\n", pathTabla);

	if((dir = opendir(pathTabla)) != NULL){
		/*Cambio el nombre de los archivos temporales de .tmp a .tmpc*/
		cambiarNombreFilesTemp(pathTabla);
		/*Agrego la tabla en el diccionario de compactación, para bloquear el acceso de las funciones que lleguen*/
		agregarTablaEnDiccCompactacion(nombreTabla);

		/*Compacto los archivos .tmpc hasta que no haya más*/
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = string_from_format(entry->d_name);
			if(string_contains(nombreArchivo, ".tmpc")){
				char* pathFile = malloc(1000 * sizeof(char));

				strcpy(pathFile, pathTabla);
				strcat(pathFile, "/");
				strcat(pathFile, "nombreArchivo");

				//FILE* temp= fopen(pathFile, "r");

/*
				while(leer(temp)!=EOF){
					//leerLinea();
					//calcularParticion();
					//agregarBloqueEnParticion();
					//escribirLineaEnBloque(); //Ver el primer bloque disponible, ver cuanto lenght queda, escribir hasta donde se pueda y lo que sigue en otro bloque
				}
*/
			}
			else{
				printf("No hay archivos temporales para compactar\n");
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
