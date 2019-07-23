/*
 * Compactador.c
 *
 *  Created on: 24 jun. 2019
 *      Author: juanmaalt
 */

#include "Compactador.h"

//Private:
//key(nro de particion), value(t_list con registros de esa particion (dump y bloque)) //Para saber si um registro partenece a tal particion => resto entre registro y cantidad de particiones de la tabla

static bool seHizoUnDump(char *pathTabla);
static EntradaDirectorio *hayArchivos(DIR *directorio);
static Metadata_tabla* levantarMetadataTabla(char *nombreTabla);
static int levantarRegistrosDump(t_dictionary *lista, char *nombreTabla, char *pathArchivoTMPC, int particionesDeLaTabla);
static int levantarRegistrosBloques(t_dictionary *lista, char *nombreTabla, int particiones);
static void agregarRegistro(t_list *lista, Registro *registro);

static void verDiccionarioDebug(t_dictionary *lista);


void* compactar(void* nombreTabla){
	pthread_detach(pthread_self());
	t_dictionary *registrosDeParticiones;
	registrosDeParticiones = dictionary_create();

	Metadata_tabla* metadata = levantarMetadataTabla((char*)nombreTabla);
	if(metadata == NULL){
		//TODO: loggear error
		return NULL;
	}

	char* pathTabla = string_from_format("%sTables/%s", config.punto_montaje, (char*)nombreTabla);
	DIR *directorio = opendir(pathTabla);
	if(directorio == NULL){
		log_error(logger_visible, "Compactador.c: compactar(%s) - Error en el path de la tabla %s, la tabla no se compactará.", (char*)nombreTabla, (char*)nombreTabla);
		log_error(logger_error, "Compactador.c: compactar(%s) - Error en el path de la tabla %s, la tabla no se compactará.", (char*)nombreTabla, (char*)nombreTabla);
		return NULL;
	}

	EntradaDirectorio *entrada = NULL;

	for( ;directorio!=NULL; directorio = opendir(pathTabla)){
		usleep(metadata->compaction_time * 1000);
		log_info(logger_invisible, "Compactador.c: compactar() - Inicio compactación de %s", pathTabla);

		if(!seHizoUnDump(pathTabla))
			continue; //Salteo y vuelvo al inicio del for a esperar a la sgte compactacion
		cambiarNombreFilesTemp(pathTabla);

		while((entrada = hayArchivos(directorio)) != NULL){
			if(!string_ends_with(entrada->d_name, ".tmpc"))
				continue; //Salteo y elijo otro archivo
			char *pathArchivoTMPC = string_from_format("%s/%s", pathTabla, entrada->d_name);
			if(levantarRegistrosDump(registrosDeParticiones, nombreTabla, pathArchivoTMPC, metadata->partitions) == EXIT_FAILURE){
				//TODO: Loggear error
				free(pathArchivoTMPC);
				continue; //Salteo y elijo al sgte archivo
			}
			free(pathArchivoTMPC);

			if(levantarRegistrosBloques(registrosDeParticiones, nombreTabla, metadata->partitions) == EXIT_FAILURE){
				continue;
			}
		}
		//TODO a partir de aca el diccionario ya esta listo para filtrarse y fueron revisados todos los TMPC
		closedir(directorio);
		verDiccionarioDebug(registrosDeParticiones);
	}

	//TODO: La compactacion de la tabla finalizo por que no se encontro el directorio
	free(metadata->consistency);
	free(metadata);
	free(pathTabla);
	return NULL;

	/*for(;;){


		if((dir = opendir(pathTabla)) != NULL){
			//Cambio el nombre de los archivos temporales de .tmp a .tmpc
			cambiarNombreFilesTemp(pathTabla);
			//Tomo el semáforo de la tabla
			waitSemaforoTabla((char*)nombreTabla);
			//Compacto los archivos .tmpc hasta que no haya más
			while((entry = readdir (dir)) != NULL){
				nombreArchivo = string_from_format(entry->d_name);
				if(string_contains(nombreArchivo, ".tmpc")){
					char* pathTemp = string_from_format("%s/%s", pathTabla, nombreArchivo);

					log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] es un archivo temporal, inciando su compactacion.", (char*)nombreTabla, nombreArchivo);
					//Leo el archivo temporal e inicio su compactación
						//printf("llega a leer temporal\n");
						//usleep(5000000);
					leerTemporal(pathTemp, metadata.partitions, (char*)nombreTabla);
						//printf("pasó leer temporal\n");
					//Borro el archivo temporal
					remove(pathTemp);
						//printf("pasó remover temp\n");
					free(pathTemp);
				}
				else{
					log_info(logger_invisible, "Compactador.c: compactar(%s): [%s] no es un archivo temporal, no se compactara", (char*)nombreTabla, nombreArchivo);
				}
				free(nombreArchivo);
			}
			//Libero el semáforo de la tabla
			postSemaforoTabla((char*)nombreTabla);
			//Lee todas las peticiones y las manda a ejecutarOperacion
			procesarPeticionesPendientes((char*)nombreTabla);
			log_info(logger_invisible, "Compactador.c: compactar(%s) - Fin compactación", (char*)nombreTabla);

			closedir (dir);
		}
	}
	config_destroy(metadataFile);
	free(pathTabla);
	return NULL;*/
}


static bool seHizoUnDump(char *pathTabla){
	DIR *dir;
	struct dirent *entry;
	char* nombreArchivo;

	if((dir = opendir(pathTabla)) != NULL){
		while((entry = readdir (dir)) != NULL){
			nombreArchivo = entry->d_name;
			if(string_ends_with(nombreArchivo, ".tmp")){
				closedir(dir);
				printf("Se hizo al menos un dump en %s\n", pathTabla);
				return true;
			}
		}
		closedir (dir);
	}
	printf("No se hizo ningun dump en %s\n", pathTabla);
	return false;
}


static EntradaDirectorio *hayArchivos(DIR *directorio){
	return readdir(directorio);
}


static Metadata_tabla* levantarMetadataTabla(char* nombreTabla){
	t_config* metadataFile = leerMetadata(nombreTabla);
	if(metadataFile == NULL){
		log_error(logger_visible, "Compactador.c: compactar(%s) - Error en el archivo 'Metadata' de la tabla %s, la tablara no se compactará.", nombreTabla, nombreTabla);
		log_error(logger_error, "Compactador.c: compactar(%s) - Error en el archivo 'Metadata' de la tabla %s, la tablara no se compactará.", nombreTabla, nombreTabla);
		return NULL;
	}
	Metadata_tabla* metadata = malloc(sizeof(Metadata_tabla));
	metadata->compaction_time = config_get_int_value(metadataFile, "COMPACTION_TIME");
	metadata->consistency = string_from_format(config_get_string_value(metadataFile, "CONSISTENCY"));
	metadata->partitions= config_get_int_value(metadataFile, "PARTITIONS");
	config_destroy(metadataFile);
	return metadata;
}


static int levantarRegistrosDump(t_dictionary *registrosDeParticiones, char *nombreTabla, char *pathArchivoTMPC, int particionesDeLaTabla){
	if(registrosDeParticiones == NULL)
		return EXIT_FAILURE;

	FILE *archivoTMPC = fopen(pathArchivoTMPC, "r");
	if(archivoTMPC == NULL)
		return EXIT_FAILURE;

	timestamp_t *timestamp = malloc(sizeof(timestamp_t));
	uint16_t *key = malloc(sizeof(uint16_t));
	char *value = malloc(sizeof(char)*(atoi(config.tamanio_value)+1)); //Va a tirar fragmentacion interna en el valgrind

	while(fscanf(archivoTMPC, "%llu;%hu;%[^\n]", timestamp, key, value)!= EOF){ //%ms significa que reserva espacio automaticamente para el value .%hu es para formatear a uint16_t
		if(strlen(value) > atoi(config.tamanio_value)){
			continue; //Lo saltea
		}
		value[strlen(value)] = '\0';

		Registro *registro = malloc(sizeof(Registro));
		registro->timestamp = *timestamp;
		registro->key = *key;
		registro->value = string_from_format(value); //No liberar ninguno de los dos hasta que se elimite el registro de la lista en el final
		free(value);
		value = malloc(sizeof(char)*(atoi(config.tamanio_value)+1));

		int particionAsignada = registro->key % particionesDeLaTabla;
		char *particionAsignadaString = string_from_format("%d", particionAsignada);

		if(!dictionary_has_key(registrosDeParticiones, particionAsignadaString)){
			t_list *registros = list_create();
			dictionary_put(registrosDeParticiones, particionAsignadaString, registros);
		}
		agregarRegistro((t_list*)dictionary_get(registrosDeParticiones, particionAsignadaString), registro);
		free(particionAsignadaString);
	}
	fclose(archivoTMPC);
	return EXIT_SUCCESS;
}


static void agregarRegistro(t_list *lista, Registro *registro){
	if(lista == NULL)
		return;
	if(registro == NULL)
		return;
	list_add(lista, registro);
}


static int levantarRegistrosBloques(t_dictionary *registrosDeParticiones, char *nombreTabla, int particiones){
	if(registrosDeParticiones == NULL)
		return EXIT_FAILURE;
	if(nombreTabla == NULL)
		return EXIT_FAILURE;

	for(int p=0; p<particiones; p++){
		char* bloquesAsignados = obtenerListaDeBloques(p, nombreTabla);

		if(bloquesAsignados == NULL){
			printf("La particion %d de la tabla %s no parece tener bloques asignados\n", p, nombreTabla);
			continue;
		}
		char** bloques = string_get_string_as_array(bloquesAsignados);
		char* particionActual= string_from_format("%d", p);
		char ch;
		char* linea = string_new();
		int i=0;

		while(bloques[i]!=NULL){
			FILE* fBloque;
			char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[i]);
			fBloque = fopen(pathBloque, "r");

			while((ch = getc(fBloque)) != EOF){
				char* nchar = string_from_format("%c", ch);
				string_append(&linea, nchar);

				if(string_ends_with(linea, "\n")){
					char** lineaParsed = string_split(linea,";");
					Registro *registro = malloc(sizeof(Registro));
					registro->timestamp = atoll(lineaParsed[0]);
					registro->key = atoi(lineaParsed[1]);
					registro->value = string_substring_until(lineaParsed[2], (strlen(lineaParsed[2])-1)); //No liberar ninguno de los dos hasta que se elimite el registro de la lista en el final

					string_iterate_lines(lineaParsed, (void* )free);
					free(lineaParsed);
					free(linea);

					if(!dictionary_has_key(registrosDeParticiones, particionActual)){
						t_list *registros = list_create();
						dictionary_put(registrosDeParticiones, particionActual, registros);
					}
					agregarRegistro((t_list*)dictionary_get(registrosDeParticiones, particionActual), registro);
					linea=string_new();
				}
				free(nchar);
			}
			i++;
		}
		free(linea);
		string_iterate_lines(bloques, (void* )free);
		free(bloques);
		free(bloquesAsignados);
		free(particionActual);

	}
	return EXIT_SUCCESS;
}





static void verDiccionarioDebug(t_dictionary *registrosDeParticiones){
	void dictionary_element_viewer(char *key, void *data){
		void list_viewer(void *registro){
			printf("Registro => timestamp: %llu, key: %hu, value: %s\n", ((Registro*)registro)->timestamp, ((Registro*)registro)->key, ((Registro*)registro)->value);
		}
		printf("Particion: %s\n", key);
		list_iterate((t_list*)data, list_viewer);
		printf("\n");
	}
	dictionary_iterator(registrosDeParticiones, dictionary_element_viewer);

}


void waitSemaforoTabla(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_wait(&(((SemaforoTabla*)semt)->semaforo));
}


void postSemaforoTabla(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_post(&(((SemaforoTabla*)semt)->semaforo));
}


