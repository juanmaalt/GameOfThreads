/*
 * FileSystem.c
 *
 *  Created on: 16 jun. 2019
 *  	Author: juanmaalt
 */

#include "FileSystem.h"


void checkEstructuraFS(){
	char option;

	printf(GREEN "Existe una estructura definida en el punto de montaje? (Y/N): " STD);

	option=getchar();
	while(option!='Y' && option!='N' && option!='y' && option!='n'){
		printf(RED "Ha ingresado un caracter no esperado. Por favor indique si existe una estructura ingresando (Y/N): ");
		option=getchar();
	}

	if(option=='N' || option=='n'){
		int blockSize=0;
		int blocks=0;
		char* magicNumber;

		printf(GREEN "Indique el tamaño de los bloques:"STD);
		scanf("%d", &blockSize);
		printf(GREEN "Indique la cantidad de bloques:"STD);
		scanf("%d", &blocks);
		printf(GREEN "Indique el magic number:"STD);
		scanf("%s", magicNumber);
		printf("magicNumber: %s\n", magicNumber);

		crearEstructuraFS(blockSize, blocks, magicNumber);
	}
}



void crearEstructuraFS(int blockSize, int blocks, char* magicNumber){

	printf("blockSize: %d\n",blockSize);
	printf("blocks: %d\n",blocks);
	printf("magicNumber: %s\n", magicNumber);

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




/*FIN FUNCIONES DIRECTORIO*/
