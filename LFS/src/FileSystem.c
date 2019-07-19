/*
 * FileSystem.c
 *
 *  Created on: 16 jun. 2019
 *  	Author: juanmaalt
 */

#include "FileSystem.h"


void checkEstructuraFS(){
	char* option;

	option=readline(GRN "Existe una estructura definida en el punto de montaje? (Y/N): " STD);
	while(!(string_equals_ignore_case(option, "Y")) && !(string_equals_ignore_case(option, "N"))){
		printf(RED "Ha ingresado un caracter no esperado '%s'. Por favor indique si existe una estructura ingresando (Y/N)" STD, option);
		option= readline(": ");
	}

	if(string_equals_ignore_case(option, "N")){
		int blockSize=atoi(readline(GRN "Indique el tamaño de los bloques: "STD));
		int blocks=atoi(readline(GRN "Indique la cantidad de bloques: "STD));
		char* magicNumber=readline(GRN "Indique el magic number: "STD);

		crearEstructuraFS(blockSize, blocks, magicNumber);
	}else{
		checkDirectorios();
	}

}



void crearEstructuraFS(int blockSize, int blocks, char* magicNumber){
	/*Creo el directorio de montaje*/
	crearDirectorioDeMontaje(config.punto_montaje);
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se creó el directorio de montaje");

	/*Creo el directorio de tablas*/
	char* pathTables = string_from_format("%sTables",config.punto_montaje);
	crearDirectorio(pathTables);
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se creó el directorio \"Tables\"");
	free(pathTables);

	/*Creo el directorio de Bloques*/
	char* pathBloques = string_from_format("%sBloques",config.punto_montaje);
	crearDirectorio(pathBloques);
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se creó el directorio \"Bloques\"");

	/*Creo los Bloques*/
	pathBloques = string_from_format("%sBloques/",config.punto_montaje);
	crearBloques(pathBloques, blocks);
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se crearon los bloques vacíos en el directorio \"Bloques\"");
	free(pathBloques);

	/*Creo el directorio de Metadata*/
	char* pathMetadata = string_from_format("%sMetadata",config.punto_montaje);
	crearDirectorio(pathMetadata);
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se creó el directorio \"Metadata\"");

	/*Creo el archivo Metadata del File System*/
	crearMetadata(pathMetadata, blockSize, blocks, magicNumber);
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se creó el archivo \"Metadata\"");

	/*Creo el bitmap para controlar los Bloques*/
	crearArchivo(pathMetadata, "/Bitmap.bin");
	log_info(logger_invisible, "FileSystem.c: crearEstructuraFS() - Se creó el archivo \"Bitmap.bin\"");
	free(pathMetadata);
}

/*INICIO FUNCIONES*/
int levantarMetadata(){

	metadata_FS=leer_MetadataFS();
	if(metadata_FS == NULL){
		RETURN_ERROR("FileSystem.c: levantarMetadata() - Error en el archivo \"Metadata.bin\"");
	}
	extraer_MetadataFS();

	return EXIT_SUCCESS;
}

t_config* leer_MetadataFS(){
	//printf("pathMetadata= %s\n",pathMetadata);

	return config_create(string_from_format("%sMetadata/Metadata.bin",config.punto_montaje));
}

void extraer_MetadataFS(){
	metadataFS.blockSize = config_get_int_value(metadata_FS, "BLOCKSIZE");
	metadataFS.blocks = config_get_int_value(metadata_FS, "BLOCKS");
}
/*FIN FUNCIONES*/

/*INICIO FUNCIONES DIRECTORIO*/
int crearDirectorio(char* path){
	DIR* dir = opendir(path);

	if (dir){
	    closedir(dir);
	}
	else if (ENOENT == errno){
		mkdir(path, 0777);
	}
	else{
		crearDirectorio(path);
	}
	return 0;
}

void crearDirectorioDeMontaje(char* puntoMontaje){
	char *subpath, *fullpath;

	fullpath = strdup(puntoMontaje);
	subpath = dirname(fullpath);

	if(strlen(subpath)>1)
		crearDirectorioDeMontaje(subpath);
	crearDirectorio(puntoMontaje);
	free(fullpath);
}

void crearBloques(char* path, int blocks){
	char filename[100];
	for(int i=1;i<blocks+1;i++){
		sprintf(filename, "%d.bin", i);
		crearArchivo(path, filename);
	}
}

void crearMetadata(char* path ,int blockSize, int blocks, char* magicNumber){
	FILE* fsMetadata;

	char* pathArchivo = string_from_format("%s/Metadata.bin", path);

	fsMetadata = fopen(pathArchivo,"a");

	fprintf (fsMetadata, "BLOCKSIZE=%d\n",blockSize);
	fprintf (fsMetadata, "BLOCKS=%d\n",blocks);
	fprintf (fsMetadata, "MAGIC_NUMBER=%s\n",magicNumber);

	fclose(fsMetadata);
	free(pathArchivo);
}

void checkExistenciaDirectorio(char* path, char* carpeta){
	DIR* dir = opendir(path);

	if (dir) {
	    closedir(dir);
	} else if (ENOENT == errno) {
		printf(RED "El directorio %s no existe. Por favor, revise el config, aseguresé de crear las carpetas necesarias y confirme nuevamente.\n" STD, carpeta);
		checkEstructuraFS();
	} else {
		checkExistenciaDirectorio(path, carpeta);
	}
}

void checkDirectorios(){
	/*Checkeo la existencia del directorio de montaje*/
	checkExistenciaDirectorio(config.punto_montaje, "de montaje");

	/*Checkeo la existencia del directorio de Tables*/
	char* pathTables = string_from_format("%sTables",config.punto_montaje);
	checkExistenciaDirectorio(pathTables, "Tables");
	free(pathTables);

	/*Checkeo la existencia del directorio de Bloques*/
	char* pathBloques = string_from_format("%sBloques",config.punto_montaje);
	checkExistenciaDirectorio(pathBloques, "Bloques");
	free(pathBloques);

	/*Checkeo la existencia del directorio de Metadata*/
	char* pathMetadata = string_from_format("%sMetadata",config.punto_montaje);
	checkExistenciaDirectorio(pathMetadata, "Metadata");
	free(pathMetadata);
}

void levantarTablasExistentes(){
	char* path = string_from_format("%sTables",config.punto_montaje);
	DIR *dir;
	struct dirent *entry;
	char* nombreCarpeta;

	if((dir = opendir(path)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreCarpeta = string_from_format(entry->d_name);
			if(!strcmp(nombreCarpeta, ".") || !strcmp(nombreCarpeta, "..")){
			}else{
				crearTablaEnMemtable(nombreCarpeta);
				log_info(logger_invisible, "FileSystem.c: levantarTablasExistentes() - Tabla levantada: %s", nombreCarpeta);
				if(agregarBloqueEnBitarray(nombreCarpeta)==0){
					iniciarCompactacion(nombreCarpeta);
				}else{
					log_error(logger_visible, "FileSystem.c: levantarTablasExistentes() - Las particiones de la Tabla \"%s\" tiene un estado inconsistente. Tabla %s borrada. ", nombreCarpeta, nombreCarpeta);
					log_error(logger_invisible, "FileSystem.c: levantarTablasExistentes() - Las particiones de la Tabla \"%s\" tiene un estado inconsistente. Tabla %s borrada. ", nombreCarpeta, nombreCarpeta);
					log_error(logger_error, "FileSystem.c: levantarTablasExistentes() - Las particiones de la Tabla \"%s\" tiene un estado inconsistente. Tabla %s borrada. ", nombreCarpeta, nombreCarpeta);
					string_append(&path, "/");
					string_append(&path, nombreCarpeta);

					int removido=removerDirectorio(path);
					if(removido!=0){
						log_error(logger_visible, "No se pudo borrar la tabla \"%s\"", nombreCarpeta);
					}
				}

			}
		}
		closedir (dir);
	}
	free(path);
}

void agregarBloqueEnParticion(char* bloque, char* nombreTabla, int particion){
	char* pathParticion = string_from_format("%sTables/%s/%d.bin", config.punto_montaje, nombreTabla, particion);
	t_config* particionData = config_create(pathParticion);

	if(particionData==NULL){
		log_info(logger_invisible, "FileSystem.c: agregarBloqueEnParticion() - No se pudo agregar el bloque %s en la particion %d de la tabla %s", bloque, particion, nombreTabla);
		log_error(logger_invisible, "FileSystem.c: agregarBloqueEnParticion() - No se pudo agregar el bloque %s en la particion %d de la tabla %s", bloque, particion, nombreTabla);
		return;
	}

	char* bloques = config_get_string_value(particionData, "BLOCKS");
	int size = config_get_int_value(particionData, "SIZE");

	char* nuevosBloques = string_from_format("%s,%s]", string_substring_until(bloques, (strlen(bloques)-1)), bloque);

	//printf("size inicial: %d\n", size);
	size = size+caracteresEnBloque(bloque);
	//printf("size inicial: %d\n", caracteresEnBloque(bloque));
	//printf("size final: %d\n", size);

	char* sizet= string_from_format("%d", size);
	config_set_value(particionData, "SIZE", sizet);
	config_set_value(particionData, "BLOCKS", nuevosBloques);

	config_save(particionData);
	config_destroy(particionData);
	free(sizet);
	free(pathParticion);
	free(nuevosBloques);
}

int agregarBloqueEnBitarray(char* nombreCarpeta){
	char* pathTablas = string_from_format("%sTables/%s/", config.punto_montaje, nombreCarpeta);

	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	if((dir = opendir(pathTablas)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = string_from_format(entry->d_name);
			if(string_contains(nombreArchivo, ".bin")){
				char* particionNbr = string_substring_until(nombreArchivo, (strlen(nombreArchivo)-4));
				//printf("antes de lista de bloques\n");
				char* listaDeBloques= obtenerListaDeBloques(atoi(particionNbr), nombreCarpeta);
				if(listaDeBloques==NULL){
					free(particionNbr);
					free(nombreArchivo);
					free(pathTablas);

					return 1;
				}else{
					//printf("post de lista de bloques\n");
					char** bloques = string_get_string_as_array(listaDeBloques);

					int i=0;

					while(bloques[i]!=NULL){
						int pos = atoi(bloques[i]);
						bitarray_set_bit(bitarray, (pos-1));
						i++;
					}
					free(particionNbr);
				}
			}
			free(nombreArchivo);
		}
	}
	free(pathTablas);
	return 0;
}
/*FIN FUNCIONES DIRECTORIO*/
