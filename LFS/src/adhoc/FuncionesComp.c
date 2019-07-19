/*
 * FuncionesComp.c
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#include "FuncionesComp.h"

void cambiarNombreFilesTemp(char* pathTabla){
	char* pathFileViejo = string_new();
	char* pathFileNuevo = string_new();
	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	if((dir = opendir(pathTabla)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = string_from_format(entry->d_name);
			if(string_contains(nombreArchivo, ".tmp")){
				pathFileViejo = string_from_format("%s/%s", pathTabla, nombreArchivo);
				pathFileNuevo = string_duplicate(pathFileViejo);
				string_append(&pathFileNuevo,"c");

				rename(pathFileViejo, pathFileNuevo);
			}
			free(nombreArchivo);
		}
		closedir (dir);
	}
	free(pathFileViejo);
	free(pathFileNuevo);
}

void leerTemporal(char* pathTemp, int particiones, char* nombreTabla){
	FILE* temp;
    int timestamp, key;
    char value[atoi(config.tamanio_value)];

    /*Abro file .tmpc*/
	temp = fopen(pathTemp, "r");
	/*Leo el file linea a linea*/
	while(fscanf(temp, "%d;%d;%s[^\n]", &timestamp, &key, value)!= EOF){
		log_info(logger_invisible, "Compactador.c: leerTemporal() - Linea leída: %d;%d;%s", timestamp, key ,value);
		char* linea = string_from_format("%d;%d;%s\n",timestamp, key, value);

		//printf("key:%d\npartciones:%d\n", key, particiones);
		char* tkey= string_from_format("%d", key);
		int particionNbr = calcularParticionNbr(tkey, particiones);
		log_info(logger_invisible, "Compactador.c: leerTemporal() - Partición Nro: %d", particionNbr);
		free(tkey);

		char* listaDeBloques= obtenerListaDeBloques(particionNbr, nombreTabla);
		log_info(logger_invisible, "Compactador.c: leerTemporal() - Bloques asignados: %s",listaDeBloques);

		if(esRegistroMasReciente(timestamp, key, listaDeBloques)){
			char* bloque = firstBloqueDisponible(listaDeBloques);

			escribirLinea(bloque, linea, nombreTabla, particionNbr);
			free(bloque);
		}
		free(linea);

	}
	fclose(temp);
}

char* obtenerListaDeBloques(int particion, char* nombreTabla){
	char* pathFile = string_from_format("%sTables/%s/%d.bin", config.punto_montaje, nombreTabla, particion);

	t_config* particionFile;
	particionFile = config_create(pathFile);
	char* listaDeBloques = string_from_format(config_get_string_value(particionFile, "BLOCKS"));

	config_destroy(particionFile);
	free(pathFile);

	return listaDeBloques;
}

char* firstBloqueDisponible(char* listaDeBloques){
	char** bloques = string_get_string_as_array(listaDeBloques);

	char* firstBloque=0;
	int i=0;

	while(bloques[i]!=NULL){
		int charsInFile = caracteresEnBloque(bloques[i]);
		//printf("Bloque %s con %d caracteres disponibles\n", bloques[i], (metadataFS.blockSize - charsInFile));
		if(charsInFile < metadataFS.blockSize){
			log_info(logger_invisible, "Compactador.c: firstBloqueDisponible() - Bloque %s con %d caracteres disponibles\n", bloques[i], (metadataFS.blockSize - charsInFile));
			firstBloque=bloques[i];
			return firstBloque;
		}
		else{
			log_info(logger_invisible, "Compactador.c: firstBloqueDisponible() - Bloque %s sin caracteres disponibles\n", bloques[i]);
			firstBloque="0";
		}
		i++;
	}
	string_iterate_lines(bloques, (void* )free);
	free(bloques);
	return firstBloque;
}

int caracteresEnBloque(char* bloque){
	char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloque);
	//printf("path: %s\n", pathBloque);

	FILE* fBloque = fopen(pathBloque, "r+");
	char ch;
	int count=0;

	while((ch=fgetc(fBloque))!=EOF){
		count++;
	}
	//printf("la cantidad de caracteres en %s.bin es %d\n", bloque, count);

	fclose(fBloque);
	free(pathBloque);
	return count;
}


void escribirEnBloque(char* bloque, char* linea){
	char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloque);

	log_info(logger_invisible, "Compactador.c: escribirEnBloque() - Path del Bloque a escribir: %s", pathBloque);

	FILE* fBloque = fopen(pathBloque, "a");

	fprintf (fBloque, "%s",linea);

	log_info(logger_invisible, "Compactador.c: escribirEnBloque() - Línea a escribir: %s", linea);

	fclose(fBloque);
	free(pathBloque);
}


void escribirLinea(char* bloque, char* linea, char* nombreTabla, int particion){
	//printf("bloque recibido: %s\n", bloque);
	if(string_equals_ignore_case(bloque, "0")){
		bloque = string_from_format("%d",getBloqueLibre());
	}
	log_info(logger_invisible, "Compactador.c: escribirLinea() - Bloque a escribir: %s", bloque);
	//printf("bloque elegido: %s\n", bloque);

	int charsDisponibles = metadataFS.blockSize-caracteresEnBloque(bloque);
	int nuevoBloque;
	char* subLinea=NULL;

	//printf("espacio en Bloque: %d\n", charsDisponibles);

	if(charsDisponibles>0){
		if(charsDisponibles>=strlen(linea)){
			escribirEnBloque(bloque, linea);
			agregarBloqueEnParticion(bloque, nombreTabla, particion);
			//printf("A> hay espacio en bloque y linea menor al espacio: linea escrita: %s\n", linea);
			//printf("A> bloque para la linea: %s\n", bloque);
		}else{
			while(strlen(linea)!=0){
				subLinea = string_substring_until(linea, charsDisponibles);
				//printf("B> hay espacio en bloque y linea mayor al espacio: linea escrita: %s\n", subLinea);
				//printf("B> nuevo bloque para la sublinea: %s\n", bloque);
				escribirEnBloque(bloque, subLinea);
				if(strlen(linea)>charsDisponibles){
					linea = string_substring_from(linea, charsDisponibles);
					//printf("B> hay espacio en bloque y linea mayor al espacio: resto: %s\n", linea);
					nuevoBloque = getBloqueLibre();
					bloque = string_from_format("%d", nuevoBloque);
					agregarBloqueEnParticion(bloque, nombreTabla, particion);
					//printf("B> nuevo bloque para la sublinea: %s\n", bloque);
				}else{linea="";}
				charsDisponibles = metadataFS.blockSize-caracteresEnBloque(bloque);
				//printf("B> actualizo char disponibles: %d\n", charsDisponibles);
			}
			//printf("B> salió\n");
		}
	}else{
		if(strlen(linea)<metadataFS.blockSize){
			nuevoBloque = getBloqueLibre();
			bloque = string_from_format("%d", nuevoBloque);
			escribirEnBloque(bloque, linea);
			agregarBloqueEnParticion(bloque, nombreTabla, particion);
			//printf("C> No hay espacio en bloque y linea menor al espacio: linea escrita: %s\n", linea);
			//printf("C> nuevo bloque para la linea: %s\n", bloque);
		}else{
			while(strlen(linea)!=0){
				nuevoBloque = getBloqueLibre();
				bloque = string_from_format("%d", nuevoBloque);
				subLinea = string_substring_until(linea, metadataFS.blockSize);
				//printf("D> No hay espacio en bloque y linea menor al espacio: linea escrita: %s\n", subLinea);
				//printf("D> nuevo bloque para la linea: %s\n", bloque);
				escribirEnBloque(bloque, subLinea);
				agregarBloqueEnParticion(bloque, nombreTabla, particion);
				linea = string_substring_from(linea, metadataFS.blockSize);
				//printf("D> No queda espacio en bloque y pido nuevo bloque: resto: %s\n", linea);
			}
		}
	}
	free(subLinea);
}

void agregarTablaEnDiccCompactacion(char* nombreTabla){
	t_list* lista = list_create();
	char* tabla=nombreTabla;

	dictionary_put(diccCompactacion, tabla, lista);
}

void procesarPeticionesPendientes(char *nombreTabla){
	t_list* listaInputsPendientes = list_create();

	t_list* inputsEnDiccionario =list_create();
	inputsEnDiccionario = dictionary_get(diccCompactacion, nombreTabla);
	listaInputsPendientes = list_take_and_remove(inputsEnDiccionario, list_size(inputsEnDiccionario));

	list_iterate(listaInputsPendientes, (void*)ejecutarOperacion);

	list_destroy(listaInputsPendientes);
}

void sacarTablaDeDiccCompactacion(char* nombreTabla){
	//dictionary_remove(diccCompactacion, nombreTabla);
	void destructorValueDicc(void * value){
		void destruirElemDeLista(void * elem){
			free((char*)elem);
		}
		list_destroy_and_destroy_elements((t_list*) value, destruirElemDeLista);

	}
	dictionary_remove_and_destroy(diccCompactacion, nombreTabla, destructorValueDicc);
}

bool esRegistroMasReciente(int timestamp, int key, char* listaDeBloques){
	Registro* reciente;

	reciente = fseekBloque(key, listaDeBloques);

	return timestamp > reciente->timestamp;
}
