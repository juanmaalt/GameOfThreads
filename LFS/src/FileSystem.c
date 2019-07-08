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
	crearArchivo(path, "/Bitmap.bin");
	/*Levantar Bitmap*/
	leerBitmap(0);

	free(path);
}

/*INICIO FUNCIONES*/
int levantarMetadata(){

	metadata_FS=leer_MetadataFS();
	if(metadata_FS == NULL){
		printf(RED"FileSystem.c: levantarMetadata: error en el archivo 'Metadata.bin'"STD"\n");
		return EXIT_FAILURE;
	}
	extraer_MetadataFS();

	return EXIT_SUCCESS;
}

t_config* leer_MetadataFS(){
	char* pathMetadata;

	char* path = malloc(1000 * sizeof(char));
	strcpy(path,config.punto_montaje);
	strcat(path, "Metadata/");
	strcat(path, "Metadata.bin");

	pathMetadata = string_from_format(path);

	free(path);

	//printf("pathMetadata= %s\n",pathMetadata);

	return config_create(pathMetadata);
}

void extraer_MetadataFS(){
	metadataFS.blockSize = config_get_int_value(metadata_FS, "BLOCKSIZE");
	metadataFS.blocks = config_get_int_value(metadata_FS, "BLOCKS");
}

void leerBitmap(int time){
	log_info(logger_invisible, "Inicio levantarBitmap");
	int size = (metadataFS.blocks/8+1);
	char* path = malloc(1000 * sizeof(char));
	strcpy(path, config.punto_montaje);
	strcat(path, "Metadata/Bitmap.bin");

	printf("path: %s\n", path);

	int fileDescriptor;
	char* bitmap;

	/*Abro el bitmap.bin, provisto o creado por la consola del fileSystem*/
	fileDescriptor = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	/*Trunco el archivo con la longitud de los bloques, para evitar problemas*/
	ftruncate(fileDescriptor, size);//
	if(fileDescriptor == -1){
		log_error(logger_visible, "No se pudo abrir el archivo");
		return;
	}
	/*Mapeo a la variable bitmap el contenido del fileDescriptor*/
	bitmap = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
	if(strlen(bitmap)==0){
		memset(bitmap,0,size);
	}
	/*Creo el bitarray para poder manejar lo leído del bitmap*/
	bitarray = bitarray_create_with_mode(bitmap, size, MSB_FIRST);

	/*Inicializo solo si es la primera vez que se abre*/
	if(time==0){
		for(int i=0;metadataFS.blocks;i++){
			bitarray_set_bit(bitarray, i);
		}
	}

	/*Sincronizo el archivo con los datos del bitarray*/
	msync(bitarray, size, MS_SYNC);

	//log_info(logger_visible, "El tamanio del bitmap es de %lu bits", tamanio);

	munmap(bitarray,size);
	close(fileDescriptor);
	free(path);

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

void levantarTablasEnMemtable(){
	char* path = malloc(1000 * sizeof(char));
	DIR *dir;
	struct dirent *entry;
	char* nombreCarpeta;

	strcpy(path,config.punto_montaje);
	strcat(path, "Tables");

	if((dir = opendir(path)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreCarpeta = string_from_format(entry->d_name);
			if(!strcmp(nombreCarpeta, ".") || !strcmp(nombreCarpeta, "..")){
			}else{
				crearTablaEnMemtable(nombreCarpeta);
				printf("Tabla levantada: %s\n", nombreCarpeta);
			}
		}
		closedir (dir);
	}
	free(path);
}

/*FIN FUNCIONES DIRECTORIO*/
