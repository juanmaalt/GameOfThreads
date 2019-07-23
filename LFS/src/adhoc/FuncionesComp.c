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
			if(string_ends_with(nombreArchivo, ".tmp")){
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
    int key;
    timestamp_t timestamp;
    char value[atoi(config.tamanio_value)];

    /*Abro file .tmpc*/
	temp = fopen(pathTemp, "r");
	/*Leo el file linea a linea*/
	while(fscanf(temp, "%llu;%d;%s[^\n]", &timestamp, &key, value)!= EOF){
		log_info(logger_invisible, "Compactador.c: leerTemporal() - Linea leída: %llu;%d;%s", timestamp, key ,value);
		char* linea = string_from_format("%llu;%d;%s\n",timestamp, key, value);

		//printf("key:%d\npartciones:%d\n", key, particiones);
		char* tkey= string_from_format("%d", key);
		int particionNbr = calcularParticionNbr(tkey, particiones);
		log_info(logger_invisible, "Compactador.c: leerTemporal() - Partición Nro: %d", particionNbr);
		free(tkey);

		char* listaDeBloques= obtenerListaDeBloques(particionNbr, nombreTabla);
		if(string_starts_with(listaDeBloques, "[")&&string_ends_with(listaDeBloques, "]")){
			log_info(logger_invisible, "Compactador.c: leerTemporal() - Bloques asignados: %s",listaDeBloques);

			if(esRegistroMasReciente(timestamp, key, listaDeBloques)){
				//printf("es más reciente\n");
				fseekAndEraseBloque(key, listaDeBloques);//TODO: descomentar
				char* bloque = firstBloqueDisponible(listaDeBloques);

				//printf("mando a escribirLinea el bloque %s\n", bloque);
				escribirLinea(bloque, linea, nombreTabla, particionNbr);
				//free(bloque);
			}
		}else{
			log_error(logger_visible, "FuncionesComp.c: leerTemporal() - Las particion '%d' de la Tabla \"%s\" tiene un estado inconsistente.", particionNbr, nombreTabla);
			log_error(logger_invisible, "FuncionesComp.c: leerTemporal() - Las particion '%d' de la Tabla \"%s\" tiene un estado inconsistente.", particionNbr, nombreTabla);
			log_error(logger_error, "FuncionesComp.c: leerTemporal() - Las particion '%d' de la Tabla \"%s\" tiene un estado inconsistente.", particionNbr, nombreTabla);
		}
		free(linea);
	}
	fclose(temp);
}

char* obtenerListaDeBloques(int particion, char* nombreTabla){
	char* pathFile = string_from_format("%sTables/%s/%d.bin", config.punto_montaje, nombreTabla, particion);

	t_config* particionFile = config_create(pathFile);
	if(particionFile == NULL)
		return NULL;
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
	if(atoi(bloque)>metadataFS.blocks){
		log_error(logger_visible, "Error a leer los caracteres del bloque '%s'", bloque);
		return metadataFS.blockSize;
	}
	char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloque);
	//printf("path: %s\n", pathBloque);

	FILE* fBloque = fopen(pathBloque, "r+");
	if(fBloque==NULL){
		log_error(logger_visible, "Error a leer los caracteres del bloque '%s'", bloque);
		free(pathBloque);
		return metadataFS.blockSize;
	}
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
	//printf("bloque que recibe escribirEnBloque: %s\n", bloque);
	char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloque);

	log_info(logger_invisible, "Compactador.c: escribirEnBloque() - Path del Bloque a escribir: %s", pathBloque);
	//printf("pathBloque - escribirEnBloque: %s\n", pathBloque);

	FILE* fBloque = fopen(pathBloque, "a");

	fprintf (fBloque, "%s",linea);

	log_info(logger_invisible, "Compactador.c: escribirEnBloque() - Línea a escribir: %s", linea);

	fclose(fBloque);
	free(pathBloque);
}


void escribirLinea(char* bloque, char* linea, char* nombreTabla, int particion){
	//printf("bloque recibido: %s\n", bloque);
	if(string_equals_ignore_case(bloque, "0")){
		char* bloqueLibre=NULL;
		if((bloqueLibre = getBloqueLibre())==NULL){
			log_error(logger_visible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
			log_error(logger_invisible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
			log_error(logger_error,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
			return;
		}

		bloque = string_from_format("%s",bloqueLibre);
		if(bloqueLibre!=NULL){
			free(bloqueLibre);
		}
	}
	log_info(logger_invisible, "Compactador.c: escribirLinea() - Bloque a escribir: %s", bloque);
	//printf("bloque elegido: %s\n", bloque);

	int charsDisponibles = metadataFS.blockSize-caracteresEnBloque(bloque);
	char* nuevoBloque;
	char* subLinea=string_new();
	subLinea=NULL;

	//printf("espacio en Bloque '%s': %d\n", bloque,charsDisponibles);

	if(charsDisponibles>0){
		if(charsDisponibles>=strlen(linea)){
			escribirEnBloque(bloque, linea);
			agregarBloqueEnParticion(bloque, nombreTabla, particion);
			//printf("A> hay espacio en bloque y linea menor al espacio: linea escrita: %s\n", linea);
			//printf("A> bloque para la linea: %s\n", bloque);
		}else{
			while(strlen(linea)!=0){
				if(strlen(linea)>charsDisponibles){
					subLinea = string_substring_until(linea, charsDisponibles);
				}else{
					subLinea = string_substring_until(linea, (strlen(linea)-1));
				}
				//printf("B> hay espacio en bloque y linea mayor al espacio: linea escrita: %s\n", subLinea);
				//printf("B> nuevo bloque para la sublinea: %s\n", bloque);
				escribirEnBloque(bloque, subLinea);
				if(strlen(linea)>charsDisponibles){
					linea = string_substring_from(linea, charsDisponibles);
					//printf("B> hay espacio en bloque y linea mayor al espacio: resto: %s\n", linea);
					if((nuevoBloque = getBloqueLibre())==NULL){
						log_error(logger_visible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
						log_error(logger_invisible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
						log_error(logger_error,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
						return;
					}
					bloque=nuevoBloque;
					agregarBloqueEnParticion(bloque, nombreTabla, particion);
					//printf("B> nuevo bloque para la sublinea: %s\n", bloque);
				}else{linea="";}
			//charsDisponibles = getMin((metadataFS.blockSize-caracteresEnBloque(bloque)), strlen(linea));
				//printf("B> actualizo char disponibles: %d\n", charsDisponibles);
			}
			//printf("B> salió\n");
		}
	}else{
		if(strlen(linea)<metadataFS.blockSize){
			if((nuevoBloque = getBloqueLibre())==NULL){
				log_error(logger_visible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
				log_error(logger_invisible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
				log_error(logger_error,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
				return;
			}
			bloque=nuevoBloque;
			escribirEnBloque(bloque, linea);
			agregarBloqueEnParticion(bloque, nombreTabla, particion);
			//printf("C> No hay espacio en bloque y linea menor al espacio: linea escrita: %s\n", linea);
			//printf("C> nuevo bloque para la linea: %s\n", bloque);
		}else{
			while(strlen(linea)!=0){
				if((nuevoBloque = getBloqueLibre())==NULL){
					log_error(logger_visible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
					log_error(logger_invisible,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
					log_error(logger_error,"FuncionesComp.c: escribirLinea() - No hay Bloques libres, no se puede guardar la información");
					return;
				}
				subLinea = string_substring_until(linea, metadataFS.blockSize);
				//printf("D> No hay espacio en bloque y linea menor al espacio: linea escrita: %s\n", subLinea);
				//printf("D> nuevo bloque para la linea: %s\n", bloque);
				bloque=nuevoBloque;
				escribirEnBloque(bloque, subLinea);
				agregarBloqueEnParticion(bloque, nombreTabla, particion);
				linea = string_substring_from(linea, metadataFS.blockSize);
				//printf("D> No queda espacio en bloque y pido nuevo bloque: resto: %s\n", linea);
			}
		}
	}
	free(subLinea);
}

void procesarPeticionesPendientes(char *nombreTabla){
	log_info(logger_visible, "Comenzando a procesar peticiones pendiente para la tabla %s", nombreTabla);
	t_list *encoladas = dictionary_get(dPeticionesPorTabla, nombreTabla);
	if(encoladas==NULL){
		return;
	}
	void procesar(void *peticion){
		Operacion op = ejecutarOperacion((char*)peticion);
		mostrarRetorno(op);
	}
	list_iterate(encoladas, procesar);
	list_destroy_and_destroy_elements(encoladas, (void*)free);
	dictionary_remove(dPeticionesPorTabla, nombreTabla);
}


bool esRegistroMasReciente(timestamp_t timestamp, int key, char* listaDeBloques){
	Registro* reciente;

	reciente = fseekBloque(key, listaDeBloques);

	//printf("timestamp recibido:%llu\ntimestamp encontrado:%llu\n", timestamp, reciente->timestamp);

	return (timestamp > reciente->timestamp);
}

int getMin(int value1, int value2){
	if(value1<value2)
		return value1;
	return value2;
}
