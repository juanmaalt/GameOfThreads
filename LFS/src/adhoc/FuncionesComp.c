/*
 * FuncionesComp.c
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#include "FuncionesComp.h"

void cambiarNombreFilesTemp(char* pathTabla){
	char* pathFileViejo = malloc(1000 * sizeof(char));
	char* pathFileNuevo = malloc(1000 * sizeof(char));
	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;



	if((dir = opendir(pathTabla)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = string_from_format(entry->d_name);
			if(string_contains(nombreArchivo, ".tmp")){
				strcpy(pathFileViejo, pathTabla);
				strcat(pathFileViejo, "/");
				strcat(pathFileViejo, nombreArchivo);
				strcpy(pathFileNuevo, pathFileViejo);
				strcat(pathFileNuevo, "c");

				rename(pathFileViejo, pathFileNuevo);
			}
		}
		closedir (dir);
	}
	free(pathFileViejo);
	free(pathFileNuevo);
}

void agregarTablaEnDiccCompactacion(char* nombreTabla){
	t_list* lista = list_create();
	char* tabla=nombreTabla;

	dictionary_put(diccCompactacion, tabla, lista);
}

void sacarTablaDeDiccCompactacion(char* nombreTabla){
	dictionary_remove(diccCompactacion, nombreTabla);
}
