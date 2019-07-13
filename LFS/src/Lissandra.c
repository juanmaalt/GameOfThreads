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
	//TODO:Meter funcion para levantar las variables de tiempo retardo y tiempo_dump

	/*Inicio el File System*/
	checkEstructuraFS();

	/*Levanto la Metadata del File System*/
	levantarMetadata();

	/*Dump*/
	//numeroDump = 0;//TODO: Checkear

	/*Inicio la Memtable*/
	memtable = inicializarDiccionario();

	//agregarDatos(memtable);//TODO:funcion para pruebas

	/*Levantar Bitmap*/
	leerBitmap();

	/*Levantar Tablas*/
	levantarTablasExistentes();

	/*Creo el diccionario para las tablas en compactación*/
	diccCompactacion = inicializarDiccionario();

	/*Inicio la consola*/
	if(iniciar_consola() == EXIT_FAILURE){
		RETURN_ERROR("Lissandra.c: main() - No se pudo levantar la consola");
	}

	/*Habilita al File System como server y queda en modo en listen*/
	int miSocket = enable_server(config.ip, config.puerto_escucha);
	log_info(logger_invisible, "Lissandra.c: main() - Servidor encendido, esperando conexiones");
	threadConnection(miSocket, connection_handler);

	/*Libero recursos*/
	config_destroy(configFile);
	dictionary_destroy(memtable);
	dictionary_destroy(diccCompactacion);
	bitarray_destroy(bitarray);

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
		log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se inició logger_error;");
		log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se inició logger_invisible;");
	}

	/*Creo logger visible*/
	logger_visible = iniciar_logger(true, "logs/LFS.log");
	if(logger_visible == NULL){
		RETURN_ERROR("Lissandra.c: configuracion_inicial() - Error al iniciar logger_visible;");
	}else{log_info(logger_invisible, "Lissandra.c: configuracion_inicial() - Se inició logger_visible;");}

	configFile = leer_config();
	if(configFile == NULL){
		RETURN_ERROR("Lissandra.c: configuracion_inicial() - Error en leer_config();");
	}
	extraer_data_config();
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
	config.retardo = config_get_string_value(configFile, "RETARDO");
	config.tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	config.tiempo_dump = config_get_string_value(configFile, "TIEMPO_DUMP");
}

void ver_config(){
	log_info(logger_visible, BLU "IP=%s" STD, config.ip);
	log_info(logger_visible, BLU "PUERTO_ESCUCHA=%s" STD, config.puerto_escucha);
	log_info(logger_visible, BLU "PUNTO_MONTAJE=%s" STD, config.punto_montaje);
	log_info(logger_visible, BLU "RETARDO=%s" STD, config.retardo);
	log_info(logger_visible, BLU "TAMANIO_VALUE=%s" STD, config.tamanio_value);
	log_info(logger_visible, BLU "TIEMPO_DUMP=%s" STD, config.tiempo_dump);
}
/*FIN FUNCIONES CONFIG*/

/*INICIO FUNCIONES COMPLEMENTARIAS*/

t_dictionary* inicializarDiccionario() {
	return dictionary_create();
}

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
			if (string_equals_ignore_case(handshake.Argumentos.TEXTO_PLANO.texto, "handshake pathLFS") == 0) {
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

	usleep(atoi(config.retardo)*1000);

	if (parsed->valido) {
		switch (parsed->keyword){
		case SELECT:
			retorno = selectAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <SELECT> Mensaje de retorno \"%s\"", retorno.Argumentos);
			break;
		case INSERT:
			retorno = insertAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <INSERT> Mensaje de retorno \"%s\"", retorno.Argumentos);
			break;
		case CREATE:
			retorno = createAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <CREATE> Mensaje de retorno \"%s\"", retorno.Argumentos);
			break;
		case DESCRIBE:
			retorno = describeAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <DESCRIBE> Mensaje de retorno \"%s\"", retorno.Argumentos);
			break;
		case DROP:
			retorno = dropAPI(*parsed);
			log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - <DROP> Mensaje de retorno \"%s\"", retorno.Argumentos);
			break;
		case RUN:
			//agregarDatos(memtable);
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
void agregarDatos(t_dictionary* memtable) {
	/*Registro* reg1 = malloc(sizeof(Registro));
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
*/
	dumpTabla("test");



}

/*FIN FUNCIONES TEST*/

/*INICIO FUNCIONES DUMP*/
/*void dump(t_dictionary* memtable) {
	//TODO: wait semaforo

	dictionary_iterator(memtable, (void*) dumpTabla);//TODO:Arreglar
	dictionary_clean(memtable);
}
*/

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

void dumpTabla(char* nombreTable){

	t_list * list = dictionary_get(memtable, nombreTable);//obtengo la data, en el insert debera checkear que este dato no sea null

	if (list == NULL || list_size(list) == 0) {
			return;
	}


	char* path = malloc(100 * sizeof(char));
	setPathTabla(path, nombreTable);

	char* pathArchivo = malloc(110 * sizeof(char));

	strcpy(pathArchivo,path);
	strcat(pathArchivo, "/dump_");
	char str[12];

	//numeroDump++;
	int numeroDump = cuentaArchivos(path);

	sprintf(str, "%d", numeroDump);
	strcat(pathArchivo, str);
	strcat(pathArchivo, ".tmp");

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
}

void dumpRegistro(FILE* file, Registro* registro) {
	fprintf(file, "%llu;%d;%s\n", registro->timestamp, registro->key, registro->value);
}
/*FIN FUNCIONES DUMP*/
