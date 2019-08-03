/*
 * FuncionesAPI.c
 *
 *  Created on: 23 jun. 2019
 *      Author: juanmaalt
 */

#include "FuncionesAPI.h"

//INICIO FUNCIONES NUEVAS
int recorrer_directorio_haciendo(char *pathDirectorio, void(*closure)(EntradaDirectorio *)){
	if(pathDirectorio == NULL)
		return EXIT_FAILURE;
	DIR *directorio = opendir(pathDirectorio);
	if(directorio == NULL)
		return EXIT_FAILURE;
	EntradaDirectorio *entrada = NULL;
	while((entrada = readdir(directorio)) != NULL)
		closure(entrada);
	closedir(directorio);
	return EXIT_SUCCESS;
}



int directory_iterate_if(char *pathDirectorio, bool (*condicion)(EntradaDirectorio*), void(*closure)(EntradaDirectorio *)){
	if(pathDirectorio == NULL)
		return EXIT_FAILURE;
	DIR *directorio = opendir(pathDirectorio);
	if(directorio == NULL)
		return EXIT_FAILURE;
	EntradaDirectorio *entrada = NULL;
	while((entrada = readdir(directorio)) != NULL)
		if(condicion(entrada))
			closure(entrada);
	closedir(directorio);
	return EXIT_SUCCESS;
}



bool directory_any_satisfy(char *pathDirectorio, bool(*closure)(EntradaDirectorio *)){
	if(pathDirectorio == NULL)
		return false;
	DIR *directorio = opendir(pathDirectorio);
	if(directorio == NULL)
		return false;
	EntradaDirectorio *entrada = NULL;
	while((entrada = readdir(directorio)) != NULL){
		if(closure(entrada)){
			closedir(directorio);
			return true;
		}
	}
	closedir(directorio);
	return false;
}

int dump_iterate_registers(char *pathDumpFile, void(*closure)(Registro*)){
	if(pathDumpFile == NULL)
		return EXIT_FAILURE;

	char* bloquesAsignados = obtenerListaDeBloquesDump(pathDumpFile);
	if(bloquesAsignados == NULL){
		log_error(logger_invisible, "El dump de la tabla no parece tener bloques asignados\n");
		return EXIT_FAILURE;
	}
	char** bloques = string_get_string_as_array(bloquesAsignados);
	char ch;
	char* linea = string_new();

	for(int i=0; bloques[i]!=NULL; i++){
		FILE* fBloque;
		char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[i]);
		fBloque = fopen(pathBloque, "r");
		if(fBloque == NULL){
			free(pathBloque);
			continue;
		}
		Registro *registro = malloc(sizeof(Registro));
		registro->value = malloc(sizeof(char)*(atoi(config.tamanio_value)+1));

		while((ch = getc(fBloque)) != EOF){
			char* nchar = string_from_format("%c", ch);
			string_append(&linea, nchar);

			if(ch =='\n'){
				char** lineaParsed = string_split(linea,";");
				if(lineaParsed != NULL && esUnRegistro(lineaParsed[0], lineaParsed[1], lineaParsed[2])){ //REVISION: Invalid read of size 4, se agrego el != NULL ahi
					registro->timestamp = atoll(lineaParsed[0]);
					registro->key = atoi(lineaParsed[1]);
					registro->value = string_substring_until(lineaParsed[2], (strlen(lineaParsed[2])-1));
					closure(registro);
					registro = malloc(sizeof(Registro));
					registro->value = malloc(sizeof(char)*(atoi(config.tamanio_value)+1));

				}
				if(lineaParsed){string_iterate_lines(lineaParsed, (void* )free); free(lineaParsed);}
				free(linea);
				linea=string_new();
			}
			free(nchar);
		}
		freopen(pathBloque, "w", stdout);
		fclose(fBloque);
		free(pathBloque);
	}
	free(linea);
	string_iterate_lines(bloques, (void* )free);
	free(bloques);
	free(bloquesAsignados);

	return EXIT_FAILURE;
}

void simple_string_iterate(char *stringToIterate, void (*closure)(char*)){
	for(int i=0; stringToIterate[i] != '\0'; ++i){
		char *characterAsString = string_from_format("%c", stringToIterate[i]);
		closure(characterAsString);
		free(characterAsString);
	}
}

/*INICIO FUNCIONES COMPLEMENTARIAS*/
bool existeTabla(char* nombreTabla){
	return dictionary_has_key(memtable, nombreTabla);
}


int getMetadata(char* nombreTabla, t_config* metadataFile){
	if(metadataFile == NULL){
		printf(RED"APILissandra.c: leerMetadata: error en el archivo 'Metadata' de la tabla %s"STD"\n", nombreTabla);
		return EXIT_FAILURE;
	}

	//extraerMetadata(metadataFile);
	//mostrarMetadata();//funcion adhoc para testing

	return EXIT_SUCCESS;
}


t_config* leerMetadata(char* nombreTabla){
	char *fullPath = string_from_format("%sTables/%s/Metadata", config.punto_montaje, nombreTabla);
	t_config* config = config_create(fullPath);
	free(fullPath);
	return config;
}

/*
void extraerMetadata(t_config* metadataFile) {
	metadata.compaction_time = config_get_int_value(metadataFile, "COMPACTION_TIME");
	metadata.consistency = config_get_string_value(metadataFile, "CONSISTENCY");
	metadata.partitions= config_get_int_value(metadataFile, "PARTITIONS");
}


void mostrarMetadata(){
	log_info(logger_visible, "\nMetadata->compaction_time= %d\n", metadata.compaction_time);
	log_info(logger_visible, "Metadata->consistency= %s\n", metadata.consistency);
	log_info(logger_visible, "Metadata->partitions= %d\n", metadata.partitions);
}
*/

t_list*	getData(char* nombreTabla){
	return dictionary_get(memtable, nombreTabla);
}


int calcularParticionNbr(char* key, int particiones){
	return atoi(key)%particiones;
}


t_list* buscarValueEnLista(t_list* data, char* key){
	//printf("Numero de partición: %d\n\n", particionNbr);

	bool compararConItem(void* item){
		if (atoi(key) != obtenerKey((Registro*) item)) {
			return false;
		}
		return true;
	}

	return list_filter(data, compararConItem);
}

void leerTemps(char* nombreTabla, char* key, t_list* listaDeValuesFiles){
	char* pathTabla = string_from_format("%sTables/%s", config.punto_montaje, nombreTabla);

	DIR *dir;
	struct dirent *entry;
	char* nombreTemp;

    if((dir = opendir(pathTabla)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreTemp = string_from_format(entry->d_name);
			if(string_contains(nombreTemp, ".tmp")){
				char* pathTemp = string_from_format("%s/%s", pathTabla, nombreTemp);
				char* listaDeBloquesDump=obtenerListaDeBloquesDump(pathTemp);
				if(string_starts_with(listaDeBloquesDump, "[")&&string_ends_with(listaDeBloquesDump, "]")){
					list_add(listaDeValuesFiles, fseekBloque(atoi(key), listaDeBloquesDump));
				}
				free(pathTemp);
				free(listaDeBloquesDump);
			}
			free(nombreTemp);
		}
		if(dir)closedir(dir);
	}
    free(pathTabla);
}


void recorrerTabla(t_list* lista){
	Registro* reg= NULL;
	int i=0;

	while(!(list_get(lista, i)==NULL)){
		reg = list_get(lista, i);

		printf("Registro->Timestamp= %llu\n", reg->timestamp);
		printf("Registro->Key= %d\n", reg->key);
		printf("Registro->Value= %s\n", reg->value);

		i++;
	}
}


void ordernarPorMasReciente(t_list* lista){
	if(list_size(lista)>0){
		if(list_size(lista)>1){
			bool compararFechas(void* item1, void* item2){
				if (obtenerTimestamp((Registro*) item1) < obtenerTimestamp((Registro*) item2)){
				return false;
				}
			return true;
			}
			list_sort(lista, compararFechas);
		}
	}
}

Registro* getMasReciente(t_list* listaDeValues, t_list* listaDeValuesFiles){
	Registro* registroMemtable = NULL;
	Registro* registroFile = NULL;

	registroMemtable = list_get(listaDeValues, 0);
	registroFile = list_get(listaDeValuesFiles, 0);

	if(registroMemtable!=NULL && registroFile!=NULL){
		if((registroMemtable->timestamp) > (registroFile->timestamp)){
			return registroMemtable;
		}else{
			return registroFile;
		}
	}else{
		if(registroMemtable!=NULL){
			return registroMemtable;
		}else{
			return registroFile;
		}
	}

}

timestamp_t checkTimestamp(char* timestamp){
	if(timestamp==NULL){
		return getCurrentTime();
	}
	else{
		//printf("time= %llu\n",atoll(timestamp));
		return atoll(timestamp);
	}
}


void crearTablaEnMemtable(char* nombreTabla){
	t_list* lista = list_create();
	char* tabla=string_from_format(nombreTabla);
	dictionary_put(memtable, tabla, lista); //la funcion hace una ocpia de la key (tabla)
	free(tabla);
}


void setPathTabla(char* path, char* nombreTabla){
	string_from_format("%sTables/%s", config.punto_montaje, nombreTabla);
}

void crearDirectorioTabla(char* path){
	crearDirectorio(path);
}


void crearArchivo(char* path, char* nombre){
	char* pathArchivo = string_from_format("%s%s", path, nombre);

	//printf("archivo creado en: %s\n", pathArchivo);

	FILE* file=NULL;
	file = fopen(pathArchivo,"w");

	fclose(file);
	free(pathArchivo);
}

void escribirArchivoMetadata(char* path, Comando comando){
	FILE* fmetadata;

	char* pathArchivo = string_from_format("%sMetadata", path);

	fmetadata = fopen(pathArchivo,"a");

	fprintf (fmetadata, "COMPACTION_TIME=%d\n",atoi(comando.argumentos.CREATE.compactacionTime));
	fprintf (fmetadata, "CONSISTENCY=%s\n",comando.argumentos.CREATE.tipoConsistencia);
	fprintf (fmetadata, "PARTITIONS=%d\n",atoi(comando.argumentos.CREATE.numeroParticiones));

	fclose(fmetadata);
	free(pathArchivo);
}

void crearArchivosBinarios(char* path, int particiones){
	FILE* binario;

	for(int i=0;i<particiones;i++){
		char* filename=string_from_format("%d.bin", i);
		//printf("path: %s\n", path);
		//printf("filename: %s\n", filename);
		crearArchivo(path, filename);

		//printf("pathArchivo: %s\n", string_from_format("%s%s", path,filename));
		char* bloque;

		if((bloque = getBloqueLibre())==NULL){
			log_error(logger_visible,"FuncionesAPI.c: crearArchivosBinarios() - No hay Bloques libres, no se puede guardar la información");
			log_error(logger_invisible,"FuncionesAPI.c: crearArchivosBinarios() - No hay Bloques libres, no se puede guardar la información");
			log_error(logger_error,"FuncionesAPI.c: crearArchivosBinarios() - No hay Bloques libres, no se puede guardar la información");
			return;
		}
		char* pathFinal=string_from_format("%s%s", path,filename);
		binario = txt_open_for_append(pathFinal);
		char* text=string_from_format("SIZE=0\nBLOCKS=[%s]\n",bloque);
		txt_write_in_file(binario, text);
		free(text);
		free(pathFinal);
		free(filename);
		free(bloque);
		fclose(binario);
	}
}

void insertInFile(char* path, int particionNbr, char* key, char* value){
	FILE* fParticion;

	char* filename=string_from_format("%d.bin", particionNbr);
	char* pathArchivo = string_from_format("%s/%s", path, filename);

	fParticion = fopen(pathArchivo,"a");

	char* keyValue = string_from_format("%s;%s", key, value);

	fprintf (fParticion, "%s",keyValue);

	fclose(fParticion);
	free(keyValue);
	free(filename);
	free(pathArchivo);
}

void getStringDescribe(char* path, char* string, char* nombreTabla, Operacion *resultadoDescribe){
	DIR *dir;
	struct dirent *entry;
	char* nombreCarpeta;
	t_config* metadata;

	if(nombreTabla==NULL){
		if((dir = opendir(path)) != NULL){
			while((entry = readdir (dir)) != NULL){
				nombreCarpeta = string_from_format(entry->d_name);
				if(!strcmp(nombreCarpeta, ".") || !strcmp(nombreCarpeta, "..")){
				}else{
					//printf("path: %s\n", pathMetadata);
					//printf("nombreTabla: %s\n", nombreCarpeta);
					char *fullPath = string_from_format("%s%s/Metadata", path, nombreCarpeta);
					metadata = config_create(fullPath);
					free(fullPath);
					if(metadata==NULL){
						log_info(logger_invisible, "No se puede acceder al archivo Metadata de la tabla %s", nombreCarpeta);
						log_error(logger_invisible, "No se puede acceder al archivo Metadata de la tabla %s", nombreCarpeta);
					}else{
						char* consistencia = string_from_format(config_get_string_value(metadata, "CONSISTENCY"));
						int compactionTime= config_get_int_value(metadata, "COMPACTION_TIME");
						int particiones = config_get_int_value(metadata, "PARTITIONS");

						concatenar_tabla(&string, nombreCarpeta, consistencia, particiones, compactionTime);
						//printf("string: %s\n", string);

						free(consistencia);
						config_destroy(metadata);
					}
				}
				free(nombreCarpeta);
		  }
			closedir(dir);
			if(strlen(string)>1){
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = string;
				resultadoDescribe->TipoDeMensaje= DESCRIBE_REQUEST;
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=true;
			}else{
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = NULL;
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=true;
			}
		}else{
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = NULL;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=true;
		}
	}else{
		if((dir = opendir (path)) != NULL){
			//printf("path: %s\n", pathMetadata);
			//printf("nombreTabla: %s\n", nombreTabla);
			char *fullPath = string_from_format("%s%s/Metadata", path, nombreTabla);
			metadata = config_create(fullPath);
			free(fullPath);
			if(metadata == NULL){
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = NULL;
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=false;
				if(dir)closedir(dir);
				return;
			}

			char* consistencia = string_from_format(config_get_string_value(metadata, "CONSISTENCY"));
			int compactionTime=config_get_int_value(metadata, "COMPACTION_TIME");
			int particiones =config_get_int_value(metadata, "PARTITIONS");

			concatenar_tabla(&string, nombreTabla, consistencia, particiones, compactionTime);
			//printf("string: %s\n", string);
			resultadoDescribe->TipoDeMensaje= DESCRIBE_REQUEST;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = string;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=false;
			config_destroy(metadata);
			free(consistencia);
		    if(dir)closedir(dir);
		}
		else{
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = NULL;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=false;
		}
	}

	//printf("string final: %s\n", string);
}

int removerDirectorio(char *path){
   DIR *dir = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (dir)   {
      struct dirent *p;
      r = 0;

      while (!r && (p=readdir(dir))){
          int r2 = -1;
          char *buf;
          size_t len;

          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")){
             continue;
          }

          len = path_len + strlen(p->d_name) + 2;
          buf = malloc(len);

          if (buf){
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);

             if (!stat(buf, &statbuf)){
                if (S_ISDIR(statbuf.st_mode)){
                   r2 = removerDirectorio(buf);
                }else{
                	r2 = unlink(buf);
                }
             }
             free(buf);
          }
          r = r2;
      }
      closedir(dir);
   }

   if (!r){
      r = rmdir(path);
   }

   return r;
}

void limpiarBloquesEnBitarray(char* nombreTabla){
	char* pathTablas = string_from_format("%sTables/%s/", config.punto_montaje, nombreTabla);

	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	if((dir = opendir(pathTablas)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = string_from_format(entry->d_name);
			if(string_contains(nombreArchivo, ".bin")){
				char* particionNbr =string_substring_until(nombreArchivo, (strlen(nombreArchivo)-4));
				char* listaDeBloques= obtenerListaDeBloques(atoi(particionNbr), nombreTabla);
				if(particionNbr) free(particionNbr); //REVISION agregado el free
				if(listaDeBloques == NULL)continue; //REVISION: agregado continue para evitar compara contra NULL.
				if(string_starts_with(listaDeBloques, "[")&&string_ends_with(listaDeBloques, "]")){
					char** bloques = string_get_string_as_array(listaDeBloques);

					int i=0;

					while(bloques[i]!=NULL){
						int pos = atoi(bloques[i]);
						bitarray_clean_bit(bitarray, (pos-1));
						i++;
					}
					if(bloques){string_iterate_lines(bloques, (void*)free); free(bloques);}//REVISION: agregado free
				}else{
					log_error(logger_visible, "FuncionesAPI.c: limpiarBloquesEnBitarray() - Las particion '%s' de la Tabla \"%s\" tiene un estado inconsistente.", particionNbr, nombreTabla);
					log_error(logger_invisible, "FuncionesAPI.c: limpiarBloquesEnBitarray() - Las particion '%s' de la Tabla \"%s\" tiene un estado inconsistente.", particionNbr, nombreTabla);
					log_error(logger_error, "FuncionesAPI.c: limpiarBloquesEnBitarray() - Las particion '%s' de la Tabla \"%s\" tiene un estado inconsistente.", particionNbr, nombreTabla);
				}
				if(listaDeBloques)free(listaDeBloques);//REVISION: agregado free listaDeBloques
			}
			free(nombreArchivo);
		}
		closedir(dir); //REVISION agregado closedir
	}
	free(pathTablas);
}

int iniciarCompactacion(char* nombreTabla, SemaforoCompactacion *semt){
	if (pthread_create(&semt->compactacionService, NULL, compactar, nombreTabla)) {
		log_error(logger_error, "FuncionesAPI.c: iniciarCompactacion(): Falló al iniciar el hilo de compactación");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/*FIN FUNCIONES COMPLEMENTARIAS*/
