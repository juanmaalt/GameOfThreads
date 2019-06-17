/*
 * FileSystem.c
 *
 *  Created on: 16 jun. 2019
 *  	Author: juanmaalt
 */

#include "FileSystem.h"


void checkEstructuraFS(){
	char option;

	printf(GRN "Existe una estructura definida en el punto de montaje? (Y/N): " STD);

	option=getchar();
	while(option!='Y' && option!='N' && option!='y' && option!='n'){
		printf(RED "Ha ingresado un caracter no esperado. Por favor indique si existe una estructura ingresando (Y/N): ");
		option=getchar();
	}

	if(option=='N' || option=='n'){
		int blockSize=0;
		int blocks=0;
		char* magicNumber;

		printf(GRN "Indique el tamaño de los bloques:"STD);
		scanf("%d", &blockSize);
		printf(GRN "Indique la cantidad de bloques:"STD);
		scanf("%d", &blocks);
		printf(GRN "Indique el magic number:"STD);
		scanf("%s", magicNumber);

		crearEstructuraFS(blockSize, blocks, magicNumber);
	}
}



void crearEstructuraFS(int blockSize, int blocks, char* magicNumber){
	char* path = malloc(100 * sizeof(char));

	/*Creo el directorio de montaje*/
	crearDirectorioDeMontaje(config.punto_montaje);
	printf("Directorio Montaje\n");

	/*Creo el directorio de tablas*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Tables");

	crearDirectorio(path);
	printf("Directorio Tables\n");

	/*Creo el directorio de Bloques*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Bloques");

	crearDirectorio(path);
	printf("Directorio Bloques\n");
	/*Creo los Bloques*/
	strcat(path, "/");
	crearBloques(path, blocks);

	/*Creo el directorio de Metadata*/
	strcpy(path,config.punto_montaje);
	strcat(path, "Metadata");

	crearDirectorio(path);
	printf("Directorio Metadata\n");
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


/*FIN FUNCIONES DIRECTORIO*/
