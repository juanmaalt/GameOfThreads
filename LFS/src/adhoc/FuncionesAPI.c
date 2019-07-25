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

int dump_iterate_registers(char *pathDumpFile, char *mode, void(*closure)(Registro*)){
	if(pathDumpFile == NULL)
		return EXIT_FAILURE;
	if(mode == NULL)
		mode = "r";
	FILE *archivo = fopen(pathDumpFile, mode);
	if(archivo == NULL)
		return EXIT_FAILURE;

	Registro *registro = malloc(sizeof(Registro));
	registro->value = malloc(sizeof(char)*(atoi(config.tamanio_value)+1));

	while(fscanf(archivo, "%llu;%hu;%[^\n]", &registro->timestamp, &registro->key, registro->value)!= EOF){
		registro->value[atoi(config.tamanio_value)] = '\0';
		closure(registro);
		registro = malloc(sizeof(Registro));
		registro->value = malloc(sizeof(char)*(atoi(config.tamanio_value)+1));
	}
	free(registro->value);
	free(registro);
	fclose(archivo);
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

void leerTemps(char* nombreTabla, char* key, t_list* listaDeValues){
	char* pathTabla = string_from_format("%sTables/%s", config.punto_montaje, nombreTabla);

	DIR *dir;
	struct dirent *entry;
	char* nombreTemp;

	FILE* temp;
    int fkey;
    timestamp_t timestamp;
    char value[atoi(config.tamanio_value)];
    if((dir = opendir(pathTabla)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreTemp = string_from_format(entry->d_name);
			if(string_contains(nombreTemp, ".tmp")){
				char* pathTemp = string_from_format("%s/%s", pathTabla, nombreTemp);
				temp = fopen(pathTemp, "r");
				while(fscanf(temp, "%llu;%d;%[^\n]s", &timestamp, &fkey, value)!= EOF){
					if(fkey==atoi(key)){
						Registro* reg = malloc(sizeof(Registro));
						reg->key = fkey;
						reg->value = string_from_format(value);
						reg->timestamp= timestamp;

						list_add(listaDeValues, reg);
					}
				}
				free(pathTemp);
			}
			free(nombreTemp);
		}
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


void getValueMasReciente(t_list* lista, Operacion* resultadoSelect){
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

		Registro* reg = list_get(lista, 0);

		resultadoSelect->TipoDeMensaje = REGISTRO;
		resultadoSelect->Argumentos.REGISTRO.timestamp=reg->timestamp;
		resultadoSelect->Argumentos.REGISTRO.key=reg->key;
		resultadoSelect->Argumentos.REGISTRO.value=string_from_format("%s",reg->value);

	}else{
		resultadoSelect->TipoDeMensaje = ERROR;
		resultadoSelect->Argumentos.ERROR.mensajeError = string_from_format("No existen registros relacionados con la key solicitada");
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
	t_list* lista = list_create(); //LEAK: ver como se libera esto
	char* tabla=string_from_format(nombreTabla);//REVISION: se agrego string from format ya que se le pasan constantes
	//TODO: a veces se la llama con nombreCarpeta, otras con nombreTabla
	dictionary_put(memtable, tabla, lista);
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
		free(bloque); //REVISION free de bloque
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

					metadata = config_create(string_from_format("%s%s/Metadata", path, nombreCarpeta));
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
			closedir (dir);
			if(strlen(string)>1){
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = string_from_format(string);
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
			if((metadata = config_create(string_from_format("%s%s/Metadata", path, nombreTabla)))==NULL){
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = NULL;
				resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=false;
				return;
			}

			char* consistencia = string_from_format(config_get_string_value(metadata, "CONSISTENCY"));
			int compactionTime=config_get_int_value(metadata, "COMPACTION_TIME");
			int particiones =config_get_int_value(metadata, "PARTITIONS");

			concatenar_tabla(&string, nombreTabla, consistencia, particiones, compactionTime);
			//printf("string: %s\n", string);
			resultadoDescribe->TipoDeMensaje= DESCRIBE_REQUEST;
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.resultado_comprimido = string_from_format(string);
			resultadoDescribe->Argumentos.DESCRIBE_REQUEST.esGlobal=false;
			config_destroy(metadata);
			free(consistencia);
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

int iniciarCompactacion(char* nombreTabla){
	pthread_t idCompactacion;
	if (pthread_create(&idCompactacion, NULL, compactar, nombreTabla)) {
		log_error(logger_error, "FuncionesAPI.c: iniciarCompactacion(): Falló al iniciar el hilo de compactación");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/*FIN FUNCIONES COMPLEMENTARIAS*/
