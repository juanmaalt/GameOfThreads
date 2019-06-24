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
	char* path = malloc(100 * sizeof(char));



	/*Creo el directorio de montaje*/
	crearDirectorioDeMontaje(config.punto_montaje);

	/*Creo el directorio de tablas*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Tables");
	crearDirectorio(path);

	/*Creo el directorio de Bloques*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Bloques");
	crearDirectorio(path);

	/*Creo los Bloques*/
	strcat(path, "/");
	crearBloques(path, blocks);

	/*Creo el directorio de Metadata*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Metadata");
	crearDirectorio(path);

	/*Creo el archivo Metadata del File System*/
	crearMetadata(path, blockSize, blocks, magicNumber);

	/*Creo el bitmap para controlar los Bloques*/

	free(path);
}


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

	char* pathArchivo = malloc(110 * sizeof(char));

	strcpy(pathArchivo,path);
	strcat(pathArchivo, "/");
	strcat(pathArchivo, "Metadata.bin");

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
	char* path = malloc(100 * sizeof(char));

	/*Checkeo la existencia del directorio de montaje*/
	checkExistenciaDirectorio(config.punto_montaje, "de montaje");

	/*Checkeo la existencia del directorio de Tables*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Tables");
	checkExistenciaDirectorio(path, "Tables");

	/*Checkeo la existencia del directorio de Bloques*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Bloques");
	checkExistenciaDirectorio(path, "Bloques");

	/*Checkeo la existencia del directorio de Bloques*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Metadata");
	checkExistenciaDirectorio(path, "Metadata");

	free(path);
}
/*FIN FUNCIONES DIRECTORIO*/
