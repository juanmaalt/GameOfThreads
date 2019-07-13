/*
 * Compactador.c
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#include "Compactador.h"

void compactar(char* nombreTabla){
	char* pathTabla = string_from_format("%sTables/%s", config.punto_montaje, nombreTabla);
	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	t_config* metadataFile = leerMetadata(nombreTabla);
	if(getMetadata(nombreTabla, metadataFile)==EXIT_FAILURE){
		log_error(logger_visible, "Compactador.c: compactar() - No existe el archivo Metadata de la tabla \"%s\". Compactación cancelada.", nombreTabla);
		log_error(logger_error, "Compactador.c: compactar() - No existe el archivo Metadata de la tabla \"%s\". Compactación cancelada.", nombreTabla);
		return;
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
				char* pathTemp = string_from_format("%s/%s", pathTabla, nombreArchivo);

			    log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] es un archivo temporal, inciando su compactación.", nombreTabla, nombreArchivo);
			    /*Leo el archivo temporal e inicio su compactación*/
			    leerTemporal(pathTemp, metadata.partitions, nombreTabla);

				remove(pathTemp);
			}
			else{
				log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] no es un archivo temporal, no se compactará", nombreTabla, nombreArchivo);
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
}

void leerTemporal(char* pathTemp, int particiones, char* nombreTabla){
	FILE* temp;
    int timestamp, key;
    char value[atoi(config.tamanio_value)];

    /*Abro file .tmpc*/
	temp = fopen(pathTemp, "r");
	/*Leo el file linea a linea*/
	while(fscanf(temp, "%d;%d;%[^\n]s", &timestamp, &key, value)!= EOF){
		log_info(logger_visible, "Compactador.c: leerTemporal() - Linea leída: %d;%d;%s\n", timestamp, key ,value);
		char* linea = string_from_format("%d;%d;%s\n",timestamp, key, value);

		int particionNbr = calcularParticionNbr(string_from_format("%d", key), particiones);
		log_info(logger_visible, "Compactador.c: leerTemporal() - Partición Nro: %d\n", particionNbr);

		char* listaDeBloques= obtenerListaDeBloques(particionNbr, nombreTabla);
		log_info(logger_visible, "Compactador.c: leerTemporal() - Bloques asignados: %s\n",listaDeBloques);

		char* bloque = firstBloqueDisponible(listaDeBloques);
		log_info(logger_visible, "Compactador.c: leerTemporal() - Primer bloque con espacio disponible: %s\n", bloque);

		escribirLinea(bloque, linea, nombreTabla, particionNbr);


	}
	fclose(temp);
}

char* obtenerListaDeBloques(int particion, char* nombreTabla){
	char* pathFile = string_from_format("%sTables/%s/%d.bin", config.punto_montaje, nombreTabla, particion);

	t_config* particionFile;
	particionFile = config_create(pathFile);
	char* resultado = config_get_string_value(particionFile, "BLOCKS");
	char* listaDeBloques = string_from_format(resultado);

	config_destroy(particionFile);

	return listaDeBloques;
}

char* firstBloqueDisponible(char* listaDeBloques){
	char** bloques = string_get_string_as_array(listaDeBloques);

	char* firstBloque=0;
	int i=0;

	while(bloques[i]!=NULL){
		int charsInFile = caracteresEnBloque(bloques[i]);
		printf("Bloque %s con %d caracteres disponibles\n", bloques[i], (metadataFS.blockSize - charsInFile));
		if(charsInFile < metadataFS.blockSize){
			log_info(logger_visible, "Compactador.c: firstBloqueDisponible() - Bloque %s con %d caracteres disponibles\n", bloques[i], (metadataFS.blockSize - charsInFile));
			firstBloque=bloques[i];
			return firstBloque;
		}
		else{
			log_info(logger_visible, "Compactador.c: firstBloqueDisponible() - Bloque %s sin caracteres disponibles\n", bloques[i]);
			firstBloque="0";
		}
		i++;
	}
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
	return count;
}


void escribirEnBloque(char* bloque, char* linea){
	char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloque);

	log_info(logger_visible, "Compactador.c: escribirEnBloque() - Path del Bloque a escribir: %s", pathBloque);

	FILE* fBloque = fopen(pathBloque, "a");

	fprintf (fBloque, "%s",linea);

	log_info(logger_visible, "Compactador.c: escribirEnBloque() - Línea a escribir: %s", linea);

	fclose(fBloque);
}


void escribirLinea(char* bloque, char* linea, char* nombreTabla, int particion){
	printf("bloque recibido: %s\n", bloque);
	if(string_equals_ignore_case(bloque, "0")){
		bloque = string_from_format("%d",getBloqueLibre());
	}
	printf("bloque elegido: %s\n", bloque);

	int charsDisponibles = metadataFS.blockSize-caracteresEnBloque(bloque);
	int nuevoBloque;
	char* subLinea=NULL;

	printf("espacio en Bloque: %d\n", charsDisponibles);

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
}


