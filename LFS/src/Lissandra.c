/*
 ============================================================================
 Name        : LFS.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Lissandra File System
 ============================================================================
 */

#include "Lissandra.h"

int main(void) {
	/*Configuración inicial para log y config*/
	if(configuracion_inicial() == EXIT_FAILURE){
		RETURN_ERROR("Lissandra.c: main() - No se pudo generar la configuracion inicial");
	}
	ver_config();

	/*Inicio el File System*/
	checkEstructuraFS();

	/*Levanto la Metadata del File System*/
	levantarMetadata();

	/*Inicio la Memtable*/
	memtable = dictionary_create();

	//agregarDatos(memtable);//Funcion para pruebas

	/*Levantar Bitmap*/
	leerBitmap();

	/*Levantar Tablas*/
	levantarTablasExistentes();

	/*Creo el diccionario para las tablas en compactación*/
	diccCompactacion = dictionary_create();

	/*Inicio la consola*/
	if(iniciar_consola() == EXIT_FAILURE){
		RETURN_ERROR("Lissandra.c: main() - No se pudo levantar la consola");
	}

	/*Inicio el hilo del Dump*/
	pthread_t idDump;
	if (pthread_create(&idDump, NULL, dump, NULL)) {
		log_error(logger_error, "Lissandra.c: main() Fallo al iniciar el hilo de Dump");
		return EXIT_FAILURE;
	}

	/*Habilita al File System como server y queda en modo en listen*/
	miSocket = enable_server(config.ip, config.puerto_escucha);
	log_info(logger_invisible, "Lissandra.c: main() - Servidor encendido, esperando conexiones");
	threadConnection(miSocket, connection_handler);

	//Rutinas de finalizacion
	rutinas_de_finalizacion();

	return EXIT_SUCCESS;
}

/*INICIO FUNCION PARA MANEJO DE HILOS*/
void *connection_handler(void *nSocket){
	pthread_detach(pthread_self());
	int socket = *(int*) nSocket;
	Operacion resultado;

	resultado = recv_msg(socket);

	log_info(logger_invisible,"Lissandra.c: connection_handler() - Nueva conexión");

	switch (resultado.TipoDeMensaje){
	case COMANDO:
		log_info(logger_invisible,"Lissandra.c: connection_handler() - Comando recibido: %s", resultado.Argumentos.COMANDO.comandoParseable);
		resultado = ejecutarOperacion(resultado.Argumentos.COMANDO.comandoParseable);
		send_msg(socket, resultado);
		break;
	case TEXTO_PLANO:
		if(strcmp(resultado.Argumentos.TEXTO_PLANO.texto, "handshake")==0)
			handshakeMemoria(socket);
		else{printf("No se pudo conectar la Memoria\n");}
		break;
	default:
		fprintf(stderr, RED"No se pude interpretar el enum %d"STD"\n", resultado.TipoDeMensaje);
	}

	destruir_operacion(resultado);

	return NULL;
}
/*FIN FUNCION PARA MANEJO DE HILOS*/

/*INICIO FUNCIONES CONFIG*/
int configuracion_inicial(){
	/*Creo la carpeta de logs*/
	mkdir("logs", 0777);
	/*Borro el último archivo de log y error.log creado*/
	remove("logs/LFS.log");
	remove("logs/LFS_error.log");

	/*Creo logger invisible de error*/
	logger_error = iniciar_logger(false, "logs/LFS_error.log");
	if(logger_error == NULL){
		RETURN_ERROR("Lissandra.c: configuracion_inicial() - Error al iniciar logger_error;");
	}

	/*Creo logger invisible*/
	logger_invisible = iniciar_logger(false, "logs/LFS.log");
	if(logger_invisible == NULL){
		RETURN_ERROR("Lissandra.c: configuracion_inicial() - Error al iniciar logger_invisible;");
	}else{
		log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se inicio logger_error;");
		log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se inicio logger_invisible;");
	}

	/*Creo logger visible*/
	logger_visible = iniciar_logger(true, "logs/LFS.log");
	if(logger_visible == NULL){
		RETURN_ERROR("Lissandra.c: configuracion_inicial() - Error al iniciar logger_visible;");
	}else{log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se inicio logger_visible;");}

	configFile = leer_config();
	if(configFile == NULL){
		RETURN_ERROR("Lissandra.c: configuracion_inicial() - Error en leer_config();");
	}
	extraer_data_config();
	extraer_data_vConfig();
	log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se extrajo la data del config;");

	return EXIT_SUCCESS;
}

t_log* iniciar_logger(bool visible, char* path) {
	return log_create(path, "LFS", visible, LOG_LEVEL_INFO);
}

t_config* leer_config() {
	return config_create(STANDARD_PATH_LFS_CONFIG);
}

void extraer_data_config() {
	config.ip = config_get_string_value(configFile, "IP");
	config.puerto_escucha = config_get_string_value(configFile, "PUERTO_ESCUCHA");
	config.punto_montaje = config_get_string_value(configFile, "PUNTO_MONTAJE");
	config.tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
}

void extraer_data_vConfig(){
	vconfig.retardo = extraer_retardo;
	vconfig.tiempoDump = extraer_tiempoDump;
}

int extraer_retardo(){
	t_config *tmpConfigFile = config_create(STANDARD_PATH_LFS_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "RETARDO");
	config_destroy(tmpConfigFile);
	return res;
}

int extraer_tiempoDump(){
	t_config *tmpConfigFile = config_create(STANDARD_PATH_LFS_CONFIG);
	int res = config_get_int_value(tmpConfigFile, "TIEMPO_DUMP");
	config_destroy(tmpConfigFile);
	return res;
}

void ver_config(){
	log_info(logger_visible, BLU "IP=%s" STD, config.ip);
	log_info(logger_visible, BLU "PUERTO_ESCUCHA=%s" STD, config.puerto_escucha);
	log_info(logger_visible, BLU "PUNTO_MONTAJE=%s" STD, config.punto_montaje);
	//log_info(logger_visible, BLU "RETARDO=%s" STD, config.retardo);
	log_info(logger_visible, BLU "TAMANIO_VALUE=%s" STD, config.tamanio_value);
	//log_info(logger_visible, BLU "TIEMPO_DUMP=%s" STD, config.tiempo_dump);
}
/*FIN FUNCIONES CONFIG*/

/*INICIO FUNCIONES COMPLEMENTARIAS*/
void handshakeMemoria(int socketMemoria) {
	log_info(logger_invisible, "Lissandra.c: handshakeMemoria() - Se conectó la memoria");

	Operacion handshake;
	handshake.TipoDeMensaje=TEXTO_PLANO;
	handshake.Argumentos.TEXTO_PLANO.texto=string_from_format(config.tamanio_value);

	/*Mando el tamanio del value*/
	send_msg(socketMemoria, handshake);

	/*Recibo un nuevo mensaje de la memoria*/
	destruir_operacion(handshake);
	handshake = recv_msg(socketMemoria);

	switch (handshake.TipoDeMensaje) {
		case TEXTO_PLANO:
			if (string_equals_ignore_case(handshake.Argumentos.TEXTO_PLANO.texto, "handshake pathLFS")) {
				destruir_operacion(handshake);
				handshake.TipoDeMensaje = TEXTO_PLANO;
				handshake.Argumentos.TEXTO_PLANO.texto=string_from_format(config.punto_montaje);
				send_msg(socketMemoria, handshake);
			}
			else{
				log_error(logger_invisible,"Lissandra.c: handshakeMemoria() - No se pudo conectar la Memoria.");
				log_error(logger_error,"Lissandra.c: handshakeMemoria() - No se pudo conectar la Memoria.");
			}
			break;
		case ERROR:
		case COMANDO:
		case REGISTRO:
		case DESCRIBE_REQUEST:
		case GOSSIPING_REQUEST:
		case GOSSIPING_REQUEST_KERNEL:
			break;
	}
	log_info(logger_invisible,"Lissandra.c: handshakeMemoria() - Memoria conectada exitosamente.");
}

int iniciar_consola(){
	if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
		RETURN_ERROR("Lissandra.c: iniciar_consola() - Falló la creación de la consola.");
	}
	log_info(logger_invisible,"Lissandra.c: iniciar_consola() - Consola levantada exitosamente.");
	return EXIT_SUCCESS;
}

Operacion ejecutarOperacion(char* input) {
	Comando *parsed = malloc(sizeof(Comando));
	Operacion retorno;
	*parsed = parsear_comando(input);

	log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - Mensaje recibido %s", input);

	usleep(vconfig.retardo()*1000);

	if (parsed->valido) {
		switch (parsed->keyword){
		case SELECT:
			/*//TODO:Meter semáforos
			if(dictionary_has_key(diccCompactacion, parsed->argumentos.SELECT.nombreTabla)){
				t_list* listaInputs;
				listaInputs=dictionary_get(diccCompactacion, parsed->argumentos.SELECT.nombreTabla);
				list_add(listaInputs, string_from_format(input));
			}else{retorno = selectAPI(*parsed);}
			*/
			retorno = selectAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <SELECT> Mensaje de retorno \"%llu;%d;%s\"", retorno.Argumentos.REGISTRO.timestamp, retorno.Argumentos.REGISTRO.key, retorno.Argumentos.REGISTRO.value);
			break;
		case INSERT:
			/*//TODO:Meter semáforos
			if(dictionary_has_key(diccCompactacion, parsed->argumentos.INSERT.nombreTabla)){
				t_list* listaInputs;
				listaInputs=dictionary_get(diccCompactacion, parsed->argumentos.INSERT.nombreTabla);
				list_add(listaInputs, string_from_format(input));
			}else{retorno = insertAPI(*parsed);}
			*/
			retorno = insertAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <INSERT> Mensaje de retorno \"%s\"", retorno.Argumentos.TEXTO_PLANO.texto);
			break;
		case CREATE:
			retorno = createAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <CREATE> Mensaje de retorno \"%s\"", retorno.Argumentos.TEXTO_PLANO.texto);
			break;
		case DESCRIBE:
			retorno = describeAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <DESCRIBE> Mensaje de retorno \"%s\"", retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
			break;
		case DROP:
			/*//TODO:Meter semáforos
			if(dictionary_has_key(diccCompactacion, parsed->argumentos.DROP.nombreTabla)){
				t_list* listaInputs;
				listaInputs=dictionary_get(diccCompactacion, parsed->argumentos.DROP.nombreTabla);
				list_add(listaInputs, string_from_format(input));
			}else{retorno = dropAPI(*parsed);}
			*/
			retorno = dropAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <DROP> Mensaje de retorno \"%s\"", retorno.Argumentos.TEXTO_PLANO.texto);
			break;
		case RUN:
			dump();
			compactar(parsed->argumentos.RUN.path);
			break;
		default:
			fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n",parsed->keyword);
		}
		destruir_comando(*parsed);
		free(parsed);
		return retorno;
	}else {
		fprintf(stderr, RED"La request no es valida"STD"\n");

		destruir_comando(*parsed);
		free(parsed);
	}

	retorno.TipoDeMensaje = ERROR;
	retorno.Argumentos.ERROR.mensajeError=malloc(sizeof(char)* (strlen("Error en la recepcion del resultado.")+1));
	strcpy(retorno.Argumentos.ERROR.mensajeError, "Error en la recepcion del resultado.");

	log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <ERROR> Mensaje de retorno \"%s\"", retorno.Argumentos);

	return retorno;
}

uint16_t obtenerKey(Registro* registro) {
		return registro->key;
}

timestamp_t obtenerTimestamp(Registro* registro) {
		return registro->timestamp;
}

/*FIN FUNCIONES COMPLEMENTARIAS*/

/*INICIO FUNCIONES TEST*/
/*
void agregarDatos(t_dictionary* memtable) {
	Registro* reg1 = malloc(sizeof(Registro));
	Registro* reg2 = malloc(sizeof(Registro));
	Registro* reg3 = malloc(sizeof(Registro));
	Registro* reg4 = malloc(sizeof(Registro));

	reg1->key = 1;
	reg1->timestamp = 1558492233084;
	reg1->value = string_from_format("pepe");

	reg2->key = 2;
	reg2->timestamp = 1558492233085;
	reg2->value = string_from_format("carlos");

	reg3->key = 3;
	reg3->timestamp = 1558492233086;
	reg3->value = string_from_format("pepe2");

	reg4->key = 4;
	reg4->timestamp = 1558492233087;
	reg4->value = string_from_format("carlos2");

	t_list* lista = list_create();

	list_add(lista,reg1);
	list_add(lista,reg2);
	list_add(lista,reg3);
	list_add(lista,reg4);

	char* tabla = string_from_format("test");

	dictionary_put(memtable, tabla, lista);//Agrego una tabla y su data;

	//lista = dictionary_get(memtable, tabla);//obtengo la data, en el insert debera checkear que este dato no sea null
}
*/
/*FIN FUNCIONES TEST*/

/*INICIO FUNCIONES DUMP*/
void* dump(){
	pthread_detach(pthread_self());

	for(;;){
		usleep(vconfig.tiempoDump() * 1000);
		dictionary_iterator(memtable, (void*) dumpTabla);
	}
	return NULL;
}

int esTemp(char* nombre) {
	char* extension = strrchr(nombre, '.');
	if(!extension || extension == nombre) return 0;
	return strcmp(extension + 1, "tmp") == 0;
}

int cuentaArchivos(char* path) {
	int cuenta = 0;
	DIR * dirp;
	struct dirent * entry;

	dirp = opendir(path);
	while ((entry = readdir(dirp)) != NULL) {
	    if ((entry->d_type == DT_REG) && (esTemp(entry->d_name) == 1)) {
		 cuenta++;
	    }
	}

	closedir(dirp);
	return cuenta;
}

void dumpTabla(char* nombreTable, t_list* list){

	//t_list * list = dictionary_get(memtable, nombreTable);//obtengo la data, en el insert debera checkear que este dato no sea null

	if (list == NULL || list_size(list) == 0) {
		return;
	}

	char* path = string_from_format("%sTables/%s", config.punto_montaje, nombreTable);

	int numeroDump = cuentaArchivos(path);

	char* pathArchivo = string_from_format("%s/D%d.tmp", path, numeroDump);

	FILE* file = fopen(pathArchivo,"w");

	Registro* reg = list_get(list, 0);

	int i = 0;
	int size = list_size(list);
	while (i < size) {
		dumpRegistro(file, reg);
		i++;
		reg = list_get(list, i);
	}
	list_clean(list);
	fclose(file);
	free(pathArchivo);
	free(path);
}

void dumpRegistro(FILE* file, Registro* registro) {
	fprintf(file, "%llu;%d;%s\n", registro->timestamp, registro->key, registro->value);
}
/*FIN FUNCIONES DUMP*/

/*INICIO FSEEK*/
Registro* fseekBloque(int key, char* listaDeBloques){
	Registro* reg = malloc(sizeof(Registro));
	reg->key = key;
	reg->value = NULL;
	reg->timestamp=0;
	char** bloques = string_get_string_as_array(listaDeBloques);

	FILE* fBloque;
	int i=0;

	char* linea = string_new();
	char ch;

	//printf("antes de while bloque\n");

	while(bloques[i]!=NULL){
		char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[i]);
		//printf("bloque[i]: %s\n", bloques[i]);
		//printf("antes de fopen: %s\n", pathBloque);
		fBloque = fopen(pathBloque, "r");
		while((ch = getc(fBloque)) != EOF){
			char* nchar = string_from_format("%c", ch);
			string_append(&linea, nchar);

			if(string_ends_with(linea, "\n")){
				char** lineaParseada = string_split(linea,";");
				if(lineaParseada[1]!=NULL){
					if(atoi(lineaParseada[1])==key){
						reg->value = string_from_format(lineaParseada[2]);
						reg->timestamp=atoll(lineaParseada[0]);

						free(pathBloque);
						free(linea);
						free(nchar);
						fclose(fBloque);
						return reg;
					}
					string_iterate_lines(lineaParseada, (void* )free);
					free(lineaParseada);
				}
			}
			free(nchar);
			//printf("linea: %s\n", linea);
		}
		fclose(fBloque);
		free(pathBloque);
		i++;
	}
	free(linea);
	return reg;
}
/*FIN FSEEK*/

/*INICIO FSEEKANDREPLACE*///TODO:Hacer

void fseekAndEraseBloque(int key, char* listaDeBloques){
	char** bloques = string_get_string_as_array(listaDeBloques);

	FILE* fBloque;
	FILE* fBloqueTemp;
	int i=0;
	int continua=0;

	char* linea = string_new();
	char ch;

	//printf("antes de while bloque\n");

	while(bloques[i]!=NULL){
		char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[i]);
		fBloque = fopen(pathBloque, "r");
		char* pathBloqueTemp = string_from_format("%sBloques/%s.binx", config.punto_montaje, bloques[i]);
		fBloqueTemp = fopen(pathBloqueTemp, "w");
		while((ch = getc(fBloque)) != EOF){
			char* nchar = string_from_format("%c", ch);
			string_append(&linea, nchar);

			if(string_ends_with(linea, "\n")){
				char** lineaParse = string_split(linea,";");
				if(lineaParse[1]!=NULL){
					if(atoi(lineaParse[1])!=key){
						if(continua!=0){
							printf("entró por continua 1 y key no es igual\n");
							char* bloqueTemp = string_from_format("%sBloques/%s.binx", config.punto_montaje, bloques[i-1]);
							remove(bloqueTemp);
							continua=0;
							free(bloqueTemp);
						}
						fprintf(fBloqueTemp, "%s", linea);
					}else{
						if(continua!=0){
							//Path del bloque viejo
							//printf("entró por continua 1 y key es igual\n");
							char* bloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[i-1]);
							//printf("bloque original: %s", bloque);
							remove(bloque);
							char* bloqueTemp = string_from_format("%sBloques/%s.binx", config.punto_montaje, bloques[i-1]);
							//printf("bloque nuevo: %s", bloqueTemp);
							rename(bloqueTemp, bloque);
							continua=0;
							free(bloque);
							free(bloqueTemp);
						}
					}

					string_iterate_lines(lineaParse, (void* )free);
					free(lineaParse);
				}
			}
			free(nchar);
			//printf("linea: %s\n", linea);
		}
		if(strlen(linea)>0){
			continua=1;
		}else{
			printf("no continua remueve\n");
			remove(pathBloqueTemp);
		}
		free(pathBloqueTemp);
		free(pathBloque);
		fclose(fBloque);
		fclose(fBloqueTemp);
		i++;
	}

	free(linea);
}
/*FIN FSEEKANDREPLACE*/



void rutinas_de_finalizacion(){
	printf(BLU"\n█▀▀▀ █▀▀█ █▀▄▀█ █▀▀ 　 █▀▀█ █▀▀ 　 ▀▀█▀▀ █░░█ █▀▀█ █▀▀ █▀▀█ █▀▀▄ █▀▀ \n█░▀█ █▄▄█ █░▀░█ █▀▀ 　 █░░█ █▀▀ 　 ░░█░░ █▀▀█ █▄▄▀ █▀▀ █▄▄█ █░░█ ▀▀█ \n▀▀▀▀ ▀░░▀ ▀░░░▀ ▀▀▀ 　 ▀▀▀▀ ▀░░ 　 ░░▀░░ ▀░░▀ ▀░▀▀ ▀▀▀ ▀░░▀ ▀▀▀░ ▀▀▀ \n\n"STD);
	log_info(logger_invisible, "=============Finalizando LFS=============");
	fflush(stdout);

	/*Libero recursos*/
	config_destroy(configFile);
	dictionary_destroy(memtable);
	dictionary_destroy(diccCompactacion);
	bitarray_destroy(bitarray);
	close(miSocket);
	log_destroy(logger_visible);
	log_destroy(logger_invisible);
	log_destroy(logger_error);

}
