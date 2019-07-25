/*
 * Bitmap.c
 *
 *  Created on: 9 jul. 2019
 *      Author: juanmaalt
 */

#include "Bitmap.h"

void leerBitmap(){
	log_info(logger_invisible, "Inicio levantarBitmap");
	size_t sizeBitarray = (metadataFS.blocks/8);
	size_t size = metadataFS.blocks;
	char* path = string_from_format("%sMetadata/Bitmap.bin", config.punto_montaje);

	//printf("path: %s\n", path);

	/*Abro el bitmap.bin, provisto o creado por la consola del fileSystem*/
	int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd == -1){
		log_error(logger_visible, "No se pudo abrir el archivo Bitmap.bin");
		log_error(logger_error, "No se pudo abrir el archivo Bitmap.bin");
		return;
	}
	/*Trunco el archivo con la longitud de los bloques, para evitar problemas*/
	ftruncate(fd, size);

		//printf("antes del mmap\n");
	/*Mapeo a la variable bitmap el contenido del fileDescriptor*/
	bitmap = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	/*Inicializo el bitmap ni bien se abre, si está vacío*/
	if(strlen(bitmap)==0){
		memset(bitmap,0,size);
	}

	//printf("antes del bitarray_create\nsize= %d\n", size);
	/*Creo el bitarray para poder manejar lo leído del bitmap*/
	bitarray = bitarray_create_with_mode(bitmap, sizeBitarray, MSB_FIRST);

	//printf("antes del msync\n");
	/*Sincronizo el archivo con los datos del bitarray*/
	msync(bitmap, size, MS_SYNC);

	//log_info(logger_visible, "El tamanio del bitmap es de %lu bits", tamanio);

	/*Función para imprimir*/
	/*
	for(size_t i=0;i<metadataFS.blocks+1;i++){
		if(bitarray_test_bit(bitarray,i)){
			printf("1");
		}else{
			printf("0");
		}
	}
	printf("\n");
	*/

	sem_init(&leyendoBitarray, 0, 1);

	close(fd);
	free(path);
}

char* getBloqueLibre(){
	sem_wait(&leyendoBitarray);

	int pos=0;
	char* posicion;

	while(bitarray_test_bit(bitarray, pos)!=0){
		if(pos<metadataFS.blocks){
			pos++;
		}else{
			log_error(logger_visible,"Bitmap.c: getBloqueLibre() - No hay Bloques libres, no se puede guardar la información");
			log_error(logger_invisible,"Bitmap.c: getBloqueLibre() - No hay Bloques libres, no se puede guardar la información");
			log_error(logger_error,"Bitmap.c: getBloqueLibre() - No hay Bloques libres, no se puede guardar la información");
			return NULL;
		}
	}

	bitarray_set_bit(bitarray, pos);

	/*Función para imprimir*/
	/*
	for(size_t i=0;i<metadataFS.blocks+1;i++){
		if(bitarray_test_bit(bitarray,i)){
			printf("1");
		}else{
			printf("0");
		}
	}
	printf("\n");
	*/

	sem_post(&leyendoBitarray);

	posicion= string_from_format("%d",pos+1);

	return posicion;
}

void liberarBloque(int pos){
	if(bitarray_test_bit(bitarray, (pos-1))){
		bitarray_clean_bit(bitarray, (pos-1));
	}

	/*Función para imprimir*/
	/*
	for(size_t i=0;i<metadataFS.blocks+1;i++){
		if(bitarray_test_bit(bitarray,i)){
			printf("1");
		}else{
			printf("0");
		}
	}
	printf("\n");
 	*/
}
