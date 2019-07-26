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
static Metadata_tabla* levantarMetadataTabla(char *nombreTabla);
static int levantarRegistrosDump(t_dictionary *lista, char *nombreTabla, char *pathArchivoTMPC, int particionesDeLaTabla);
static int levantarRegistrosBloques(t_dictionary *lista, char *nombreTabla, int particiones);
static void agregarRegistro(t_list *lista, Registro *registro);
static void verDiccionarioDebug(t_dictionary *lista);
static void destruirRegistrosDeParticiones(t_dictionary *diccionario);
static int escribirDiccionarioEnBloques(t_dictionary* registrosDeParticiones, char* nombreTabla);
static int escribirBloques(t_list* listaDeRegistros, char** bloques, char* nombreTabla, int particion);


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

	bool es_tmpc(EntradaDirectorio *entrada){
		return string_ends_with(entrada->d_name, ".tmpc");
	}

	void iterar_tmpc(EntradaDirectorio *entrada){
		char *pathArchivoTMPC = string_from_format("%s/%s", pathTabla, entrada->d_name);
		levantarRegistrosDump(registrosDeParticiones, nombreTabla, pathArchivoTMPC, metadata->partitions);
		free(pathArchivoTMPC);
	}

	for(;;){
		usleep(metadata->compaction_time * 1000);
		log_info(logger_invisible, "Compactador.c: compactar() - Inicio compactación de %s", pathTabla);
		if(!seHizoUnDump(pathTabla))
			continue;
		cambiarNombreFilesTemp(pathTabla);
		if(levantarRegistrosBloques(registrosDeParticiones, (char*)nombreTabla, metadata->partitions) == EXIT_FAILURE){
			//TODO: Loggear el error
			continue;
		}
		if(directory_iterate_if(pathTabla, es_tmpc, iterar_tmpc)==EXIT_FAILURE){
			//TODO: Loggear el error
			continue;
		}
		verDiccionarioDebug(registrosDeParticiones);
		escribirDiccionarioEnBloques(registrosDeParticiones, (char*)nombreTabla);
		destruirRegistrosDeParticiones(registrosDeParticiones);
		//TODO: Destruir temps
		procesarPeticionesPendientes(nombreTabla);
		destruirPeticionesPendientes(nombreTabla);
	}

	return NULL;
}





static bool seHizoUnDump(char *pathTabla){
	bool es_tmp(EntradaDirectorio *entrada){
		return string_ends_with(entrada->d_name, ".tmp");
	}
	return directory_any_satisfy(pathTabla, es_tmp);
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

	bool existeLaParticion(char *particion){
		return dictionary_has_key(registrosDeParticiones, particion);
	}

	bool estaLaKey(t_list *list, uint16_t key){
		bool keyIgual(void *registro){
			return ((Registro*)registro)->key == key;
		}
		return list_any_satisfy(list, keyIgual);
	}

	bool hayQueAgregarlo(Registro *registro, t_list *lista){
		bool keyIgual(void *elemento){
			return ((Registro*)elemento)->key == registro->key;
		}
		bool sonViejos(void *elemento){
			return ((Registro*)elemento)->timestamp <= registro->timestamp;
		}
		t_list *keysIguales = list_filter(lista, keyIgual);
		bool retorno = list_all_satisfy(keysIguales, sonViejos);
		list_destroy(keysIguales);
		return retorno;
	}

	void removerKeysIguales(t_list *registrosDeParticion, uint16_t key){
		bool keyIgual(void *elemento){
			return ((Registro*)elemento)->key == key;
		}
		void destruir(void *elemento){
			free(((Registro*)elemento)->value);
			free((Registro*)elemento);
		}
		list_remove_and_destroy_by_condition(registrosDeParticion, keyIgual, destruir);
	}

	void levantarRegistro(Registro *registro){
		char *particionAsignada = string_from_format("%d", registro->key % particionesDeLaTabla);
		//Step 1: check partition
		if(!existeLaParticion(particionAsignada)){
			t_list *registrosDeParticion = list_create();
			list_add(registrosDeParticion, registro);
			dictionary_put(registrosDeParticiones, particionAsignada, registrosDeParticion);
			free(particionAsignada);
			return;
		}
		//Step 2: if partition exists, check if exists the key in partition
		t_list *registrosDeParticion = dictionary_get(registrosDeParticiones, particionAsignada);
		if(!estaLaKey(registrosDeParticion, registro->key)){
			list_add(registrosDeParticion, registro);
			free(particionAsignada);
			return;
		}
		//Step 3: if partition exists and has the key, compare and add the newest register
		if(hayQueAgregarlo(registro, registrosDeParticion)){
			removerKeysIguales(registrosDeParticion, registro->key);
			list_add(registrosDeParticion, registro);
			free(particionAsignada);
			return;
		}
		//No cumplio ninguna condicion => no se uso para nada y ya no lo necesitamos
		free(registro->value);
		free(registro);
		free(particionAsignada);
	}

	if(dump_iterate_registers(pathArchivoTMPC, "r", levantarRegistro) == EXIT_FAILURE)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
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

		for(int i=0; bloques[i]!=NULL; i++){
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
		}
		free(linea);
		string_iterate_lines(bloques, (void* )free);
		free(bloques);
		free(bloquesAsignados);
		free(particionActual);

	}
	return EXIT_SUCCESS;
}





static void agregarRegistro(t_list *lista, Registro *registro){
	if(lista == NULL)
		return;
	if(registro == NULL)
		return;
	list_add(lista, registro);
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






static void destruirRegistrosDeParticiones(t_dictionary *diccionario){
	void destruirDiccionario(void *lista){
		void destruirLista(void *registro){
			free(((Registro*)registro)->value);
			free((Registro*)registro);
		}
		list_destroy_and_destroy_elements((t_list*)lista, destruirLista);
	}
	dictionary_destroy_and_destroy_elements(diccionario, destruirDiccionario);
}



void waitSemaforoTabla(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_wait(&(((SemaforoTabla*)semt)->semaforoGral));
}


void waitSemaforoTablaSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_wait(&(((SemaforoTabla*)semt)->semaforoSelect));
	((SemaforoTabla*) semt)->inicioBloqueo = getCurrentTime();
	log_info(logger_visible, "Compactar(%s): Se inicio la escritura de los bloques de la tabla %s", tabla, tabla);
}




void postSemaforoTabla(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_post(&(((SemaforoTabla*)semt)->semaforoGral));
}

void postSemaforoTablaSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_post(&(((SemaforoTabla*)semt)->semaforoSelect));
	((SemaforoTabla*) semt)->finBloqueo = getCurrentTime();
	log_info(logger_visible, "Compactar(%s): Se finalizo la escritura de los bloques de la tabla %s", tabla, tabla);
	log_info(logger_visible, "Compactar(%s): Tiempo en el que la tabla estuvo bloqueada= %llu", tabla, (((SemaforoTabla*) semt)->finBloqueo-((SemaforoTabla*) semt)->inicioBloqueo));
}


static int escribirDiccionarioEnBloques(t_dictionary* registrosDeParticiones, char* nombreTabla){
	for(int i=0; i<dictionary_size(registrosDeParticiones);i++){
		char* particion = string_from_format("%d", i);
		char* bloquesAsignados = obtenerListaDeBloques(i, nombreTabla);
		char** bloques = string_get_string_as_array(bloquesAsignados);
		t_list* listaDeRegistros = dictionary_get(registrosDeParticiones, particion);

		escribirBloques(listaDeRegistros, bloques, nombreTabla, i);

		free(particion);
		string_iterate_lines(bloques, (void* )free);
		free(bloques);
		free(bloquesAsignados);
	}
	return EXIT_SUCCESS;
}

static int escribirBloques(t_list* listaDeRegistros, char** bloques, char* nombreTabla, int particion){
	int size = metadataFS.blockSize;
	int numeroDeBloque = 0;
	char **registrosBloques = generarRegistroBloque(listaDeRegistros);

	void escribirEnBloques(char *linea){
		char* pathBloque;
		if(bloques[numeroDeBloque]!=NULL){
			pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[numeroDeBloque]);
			numeroDeBloque++;
		}else{
			char* bloque = getBloqueLibre();
			pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje,bloque);
			agregarBloqueEnParticion(bloque, nombreTabla, particion);
			free(bloque);
		}
		int fdBloque = open(pathBloque, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if(fdBloque == -1){
			log_error(logger_visible, "No se pudieron abrir los bloques de la particion");
			log_error(logger_error, "No se pudieron abrir los bloques de la particion");
			free(pathBloque);
			close(fdBloque);
			return;
		}
		ftruncate(fdBloque, strlen(linea)); //strlen linea va a ser menor o igual a size. Si lo hago por el size, y nosotros escribimos menos del size, entonces se va a ver obligado a rellenar el resto del archivo con caracteres feos
		char* textoBloque = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdBloque, 0);
		memcpy(textoBloque, linea, strlen(linea)); //strlen linea va a ser menor o igual a size. La linea contiene \n al finalizar cada registro. Contiene \0 al final pero el strlen no lo tiene en cuenta
		msync(textoBloque, size, MS_SYNC);
		free(pathBloque);
		munmap(textoBloque, size);
		close(fdBloque);
	}
	string_iterate_lines(registrosBloques, escribirEnBloques);
	if(registrosBloques){
		string_iterate_lines(registrosBloques, (void*)free);
		free(registrosBloques);
	}
	return EXIT_SUCCESS;
}


