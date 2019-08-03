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
static void *inotify_service(void *null);

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

	/*Creo estructuras para semaforos*/
	iniciar_semaforos();

	/*Levantar Tablas*/
	levantarTablasExistentes();

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
	Operacion recibido;
	Operacion resultado;

	recibido = recv_msg(socket);

	log_info(logger_invisible,"Lissandra.c: connection_handler() - Nueva conexión");

	switch (recibido.TipoDeMensaje){
	case COMANDO:
		log_info(logger_invisible,"Lissandra.c: connection_handler() - Comando recibido: %s", recibido.Argumentos.COMANDO.comandoParseable);
		resultado = ejecutarOperacion(recibido.Argumentos.COMANDO.comandoParseable);
		send_msg(socket, resultado);
		destruir_operacion(resultado);
		break;
	case HANDSHAKE:
		if(strcmp(recibido.Argumentos.HANDSHAKE.informacion, "handshake")==0)
			handshakeMemoria(socket);
		else
			log_error(logger_visible ,"Lissandra.c: connectionHandler() - No se pudo conectar la Memoria.");
			log_error(logger_error ,"Lissandra.c: connectionHandler() - No se pudo conectar la Memoria.");
		break;
	default:
		fprintf(stderr, RED"No se pude interpretar el enum %d"STD"\n", recibido.TipoDeMensaje);
	}
	destruir_operacion(recibido);
	close(socket);
	free(nSocket);
	return NULL;
}
/*FIN FUNCION PARA MANEJO DE HILOS*/

/*INICIO FUNCIONES CONFIG*/
int configuracion_inicial(){
	/*Creo la carpeta de logs*/
	mkdir("logs", 0777);
	/*Borro el último archivo de error.log creado*/
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
	refrescar_vconfig();
	config_destroy(configFile);
	if(pthread_create(&inotify, NULL, inotify_service, NULL))
		RETURN_ERROR("Lissandra.c: configuracion_inicial: no se pudo iniciar inotify");
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
	if(config_get_string_value(configFile, "IP") != NULL)
		config.ip = string_from_format(config_get_string_value(configFile, "IP"));
	if(config_get_string_value(configFile, "PUERTO_ESCUCHA") != NULL)
		config.puerto_escucha = string_from_format(config_get_string_value(configFile, "PUERTO_ESCUCHA"));
	if(config_get_string_value(configFile, "PUNTO_MONTAJE") != NULL)
		config.punto_montaje = string_from_format(config_get_string_value(configFile, "PUNTO_MONTAJE"));
	if(config_get_string_value(configFile, "TAMANIO_VALUE") != NULL)
		config.tamanio_value = string_from_format(config_get_string_value(configFile, "TAMANIO_VALUE"));
}

#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
static void *inotify_service(void *null){
	void *service(){
		int fd = inotify_init();
		if(fd<0){
			printf(RED"Lissandra.c: inotify_service: fallo el fd para inotify\n"STD);
			return NULL;
		}

		int watch = inotify_add_watch(fd, STANDARD_PATH_LFS_CONFIG, IN_MODIFY | IN_DELETE);
		if(watch<0){
			printf(RED"Lissandra.c: inotify_service: fallo en el add watch para inotify\n"STD);
			return NULL;
		}
		char buf[BUF_LEN];
		int len, i = 0;
		len = read (fd, buf, BUF_LEN);
		while (i < len) {
			struct inotify_event *event;
			event = (struct inotify_event *) &buf[i];
			refrescar_vconfig();
			log_info(logger_visible, GRN"El archivo de configuracion ha cambiado"STD);
			log_info(logger_invisible, "El archivo de configuracion ha cambiado");
			ver_config();
			//printf("wd=%d mask=%u cookie=%u len=%u\n", event->wd, event->mask, event->cookie, event->len);
			i += EVENT_SIZE + event->len;
		}
		return NULL;
	}

	for(;;)
		service();

	printf(YEL"Lissandra.c: inotify_service: inotify finalizo. Ya no se podra tener un seguimiento del archivo de configuracion.\n"STD);
	return NULL;
}

void refrescar_vconfig(){
	t_config *configFile = config_create(STANDARD_PATH_LFS_CONFIG);
	if(configFile == NULL){
		log_error(logger_visible, "inotify_service: refrescar_vconfig: el archivo de configuracion no se encontro");
		log_error(logger_invisible, "inotify_service: refrescar_vconfig: el archivo de configuracion no se encontro");
	}

	void error(char* m){
		log_error(logger_visible, "%s", m);
		log_error(logger_invisible, "%s", m);
	}

	if(config_get_string_value(configFile, "RETARDO") == NULL)
		error("Lissandra.c: inicializar_configs: error en la extraccion del parametro RETARDO");
	else if(!esNumerica(config_get_string_value(configFile, "RETARDO"), false))
		error("Lissandra.c: inicializar_configs: el parametro RETARDO debe ser numerico");
	else{
		sem_wait(&mutexVConfig);
		vconfig.retardo = config_get_int_value(configFile, "RETARDO");
		sem_post(&mutexVConfig);
	}

	if(config_get_string_value(configFile, "TIEMPO_DUMP") == NULL)
		error("Lissandra.c: inicializar_configs: error en la extraccion del parametro TIEMPO_DUMP");
	else if(!esNumerica(config_get_string_value(configFile, "TIEMPO_DUMP"), false))
		error("Lissandra.c: inicializar_configs: el parametro TIEMPO_DUMP debe ser numerico");
	else{
		sem_wait(&mutexVConfig);
		vconfig.tiempoDump = config_get_int_value(configFile, "TIEMPO_DUMP");
		sem_post(&mutexVConfig);
	}

	config_destroy(configFile);
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
	log_info(logger_invisible, "Lissandra.c: handshakeMemoria() - La Memoria está intentando conectarse");

	Operacion handshake;
	handshake.TipoDeMensaje=HANDSHAKE;
	handshake.Argumentos.HANDSHAKE.informacion=string_from_format(config.tamanio_value);

	/*Mando el tamanio del value*/
	send_msg(socketMemoria, handshake);

	/*Recibo un nuevo mensaje de la memoria*/
	destruir_operacion(handshake);
	handshake = recv_msg(socketMemoria);

	switch (handshake.TipoDeMensaje){
		case HANDSHAKE:
			if (string_equals_ignore_case(handshake.Argumentos.HANDSHAKE.informacion, "handshake pathLFS")) {
				destruir_operacion(handshake);
				handshake.TipoDeMensaje = HANDSHAKE;
				handshake.Argumentos.HANDSHAKE.informacion=string_from_format(config.punto_montaje);
				send_msg(socketMemoria, handshake);
			}
			else{
				log_error(logger_visible,"Lissandra.c: handshakeMemoria() - No se pudo conectar la Memoria.");
				log_error(logger_error,"Lissandra.c: handshakeMemoria() - No se pudo conectar la Memoria.");
				destruir_operacion(handshake);
			}
			break;
		default:
			break;
	}
	destruir_operacion(handshake);
	log_info(logger_visible,"Lissandra.c: handshakeMemoria() - Memoria conectada exitosamente.");
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

	log_info(logger_visible,"Lissandra.c: ejecutarOperacion() - Mensaje recibido %s", input);

	sem_wait(&mutexVConfig);
	int retardo = vconfig.retardo;
	sem_post(&mutexVConfig);
	usleep(retardo*1000);

	if (parsed->valido) {
		switch (parsed->keyword){
		case SELECT:
			tryExecuteSelect(parsed->argumentos.SELECT.nombreTabla);
			seVaAEjecutarRequestSelect(parsed->argumentos.SELECT.nombreTabla);
			retorno = selectAPI(*parsed);
			seTerminoDeEjecutarRequestSelect(parsed->argumentos.SELECT.nombreTabla);
			break;
		case INSERT:
			tryExecute(parsed->argumentos.INSERT.nombreTabla);
			retorno = insertAPI(*parsed);
			break;
		case CREATE:
			retorno = createAPI(*parsed);
			break;
		case DESCRIBE:
			retorno = describeAPI(*parsed);
			break;
		case DROP:
			tryExecute(parsed->argumentos.DROP.nombreTabla);
			retorno = dropAPI(*parsed);
			break;
		case RUN:
			//liberarBloque(atoi(parsed->argumentos.RUN.path));
			//compactar(parsed->argumentos.RUN.path);
			break;
		case JOURNAL:
			//dump();
			//getBloqueLibre();
			break;
		default:
			fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n",parsed->keyword);
		}
		destruir_comando(*parsed);
		free(parsed);
		mostrarRetorno(retorno);
		return retorno;
	}else {
		fprintf(stderr, RED"La request no es valida"STD"\n");

		destruir_comando(*parsed);
		free(parsed);
	}

	retorno.TipoDeMensaje = ERROR;
	retorno.Argumentos.ERROR.mensajeError=string_from_format("Error en la recepcion del resultado.");
	log_error(logger_visible,"<ERROR>\n%s",retorno.Argumentos.ERROR.mensajeError);

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
		sem_wait(&mutexVConfig);
		int tiempoDump=vconfig.tiempoDump;
		sem_post(&mutexVConfig);
		usleep(tiempoDump* 1000);
		sem_wait(&mutexAgregarTablaMemtable);
		dictionary_iterator(memtable, dumpTabla);
		sem_post(&mutexAgregarTablaMemtable);
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
	if(dirp == NULL)
		return -1;
	while ((entry = readdir(dirp)) != NULL) {
	    if ((entry->d_type == DT_REG) && (esTemp(entry->d_name) == 1)) {
		 cuenta++;
	    }
	}

	closedir(dirp);
	return cuenta;
}

void dumpTabla(char* nombreTable, void* value){
	sem_wait(&mutexMemtable);
	t_list* list = (t_list*) value;
	if(list==NULL || list_size(list)==0){
		log_info(logger_invisible, "Lissandra.c: dumpTabla() - No hay datos para Dumpear para la tabla %s", nombreTable);
		sem_post(&mutexMemtable);
		return;
	}
	sem_post(&mutexMemtable);

	char* path = string_from_format("%sTables/%s", config.punto_montaje, nombreTable);

	int numeroDump = cuentaArchivos(path);
	if(numeroDump == -1) return; //REVISION -1

	char* pathArchivo = string_from_format("%s/D%d.tmp", path, numeroDump);

	FILE *dump = fopen(pathArchivo, "w");

	dump = txt_open_for_append(pathArchivo);
	char* text=string_from_format("SIZE=0\nBLOCKS=[]\n");
	txt_write_in_file(dump, text);
	free(text);

	escribirBloquesDump(list, nombreTable, pathArchivo);

	fclose(dump);
	free(pathArchivo);
	free(path);

	void eliminarRegistro(void* elem){
		if(((Registro*)elem)->value!=NULL){
			free(((Registro*)elem)->value);
		}
		free(((Registro*)elem));
	}

	sem_wait(&mutexMemtable);
	list_clean_and_destroy_elements(list, eliminarRegistro);
	sem_post(&mutexMemtable);
}

void dumpRegistro(FILE* file, Registro* registro) {
	fprintf(file, "%llu;%d;%s\n", registro->timestamp, registro->key, registro->value);
}

int escribirBloquesDump(t_list* listaDeRegistros, char* nombreTabla, char* pathArchivo){
	int size = metadataFS.blockSize;
	sem_wait(&mutexMemtable);
	char **registrosBloques = generarRegistroBloque(listaDeRegistros);
	sem_post(&mutexMemtable);
	char* pathBloque;

	void escribirEnBloquesDump(char *linea){
		char* bloque = getBloqueLibre();
		if(atoi(bloque)==0){
			log_error(logger_invisible, "No hay mas bloques disponibles");
			log_error(logger_error, "No hay mas bloques disponibles");
			return;
		}
		pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje,bloque);
		agregarBloqueEnDump(bloque, nombreTabla, pathArchivo);
		free(bloque);
		int fdBloque = open(pathBloque, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if(fdBloque == -1){
			log_error(logger_visible, "No se pudieron abrir los bloques");
			log_error(logger_error, "No se pudieron abrir los bloques");
			free(pathBloque);
			close(fdBloque);
			if(registrosBloques){
				string_iterate_lines(registrosBloques, (void*)free);
				free(registrosBloques);
			}
			return;
		}
		ftruncate(fdBloque, strlen(linea));
		char* textoBloque = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdBloque, 0);
		memcpy(textoBloque, linea, strlen(linea));
		msync(textoBloque, size, MS_SYNC);
		free(pathBloque);
		munmap(textoBloque, size);
		actualizarTamanioEnDump(strlen(linea), nombreTabla, pathArchivo);
		close(fdBloque);
	}
	string_iterate_lines(registrosBloques, escribirEnBloquesDump);
	if(registrosBloques){
		string_iterate_lines(registrosBloques, (void*)free);
		free(registrosBloques);
	}
	return EXIT_SUCCESS;
}

/*FIN FUNCIONES DUMP*/

/*INICIO FSEEK*/
Registro* fseekBloque(int key, char* listaDeBloques){
	Registro* reg = malloc(sizeof(Registro));
	reg->key = key;
	reg->value = NULL;
	reg->timestamp=0;
	char** bloques = string_get_string_as_array(listaDeBloques); //REVISION: se libera esto al final de la funcion

	FILE* fBloque;
	int i=0;

	char* linea = string_new();
	char ch;

	void liberarArrayString(char **array){
		if(array != NULL){
			string_iterate_lines(array, (void*)free);
			free(array);
		}
	}

	//printf("antes de while bloque\n");

	while(bloques[i]!=NULL){
		char* pathBloque = string_from_format("%sBloques/%s.bin", config.punto_montaje, bloques[i]);
		//printf("bloque[i]: %s\n", bloques[i]);
		//printf("antes de fopen: %s\n", pathBloque);
		fBloque = fopen(pathBloque, "r");
		while((ch = getc(fBloque)) != EOF){
			char* nchar = string_from_format("%c", ch);
			string_append(&linea, nchar);
			free(nchar);
			if(ch == '\n'){
				char** lineaParseada = string_split(linea,";");
				//printf("linea encontrada: %s\n", linea);
				if(lineaParseada[1]!=NULL){
					if(atoi(lineaParseada[1])==key){
						reg->value = string_from_format(lineaParseada[2]);
						reg->timestamp=atoll(lineaParseada[0]);

						free(pathBloque);
						free(linea);
						fclose(fBloque);
						liberarArrayString(bloques);
						liberarArrayString(lineaParseada);
						return reg;
					}
				}
				liberarArrayString(lineaParseada);
				if(linea != NULL)free(linea);
				linea = string_new();
			}
			//printf("linea: %s\n", linea);
		}
		fclose(fBloque);
		free(pathBloque);
		i++;
	}
	liberarArrayString(bloques);
	if(linea!=NULL)free(linea);
	return reg;
}
/*FIN FSEEK*/

void rutinas_de_finalizacion(){
	printf(BLU"\n  #####                               #######         \n #     #    ##    #    #  ######      #     #  ###### \n #         #  #   ##  ##  #           #     #  #      \n #  ####  #    #  # ## #  #####       #     #  #####  \n #     #  ######  #    #  #           #     #  #      \n #     #  #    #  #    #  #           #     #  #      \n  #####   #    #  #    #  ######      #######  #      \n"STD);
	printf("\n");
	printf(BLU"#######                                                 \n   #     #    #  #####   ######    ##    #####    ####  \n   #     #    #  #    #  #        #  #   #    #  #      \n   #     ######  #    #  #####   #    #  #    #   ####  \n   #     #    #  #####   #       ######  #    #       # \n   #     #    #  #   #   #       #    #  #    #  #    # \n   #     #    #  #    #  ######  #    #  #####    ####   \n\n"STD);
	log_info(logger_invisible, "=============Finalizando LFS=============");
	fflush(stdout);

	/*Libero recursos*/
	dictionary_destroy(memtable);
	bitarray_destroy(bitarray);
	close(miSocket);
	munmap(bitmap,metadataFS.blocks);
	log_destroy(logger_visible);
	log_destroy(logger_invisible);
	log_destroy(logger_error);

}
