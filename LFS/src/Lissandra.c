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

	/*Creo el diccionario para las tablas en compactación*/
	dPeticionesPorTabla = dictionary_create();//REVISION: se liberan las peticiones cuando se hace un drop
	semaforosPorTabla =list_create();
	sem_init(&mutexPeticionesPorTabla, 0, 1);

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
	case COMANDO://TODO checkear que funcione
		log_info(logger_invisible,"Lissandra.c: connection_handler() - Comando recibido: %s", recibido.Argumentos.COMANDO.comandoParseable);
		resultado = ejecutarOperacion(recibido.Argumentos.COMANDO.comandoParseable);
		send_msg(socket, resultado);
		destruir_operacion(resultado);
		break;
	case HANDSHAKE:
		if(strcmp(recibido.Argumentos.HANDSHAKE.informacion, "handshake")==0) //TODO: crear handshake memoria
			handshakeMemoria(socket);
		else
			log_error(logger_visible ,"Lissandra.c: connectionHandler() - No se pudo conectar la Memoria.");
			log_error(logger_error ,"Lissandra.c: connectionHandler() - No se pudo conectar la Memoria.");
		break;
	default:
		fprintf(stderr, RED"No se pude interpretar el enum %d"STD"\n", recibido.TipoDeMensaje);
	}
	destruir_operacion(recibido);

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
	int valorSemaforo=0;

	SemaforoTabla *semt=NULL;
	char *bufferTabla=NULL;
	bool buscar(void *tablaSemaforo){
		return string_equals_ignore_case(bufferTabla, ((SemaforoTabla*) tablaSemaforo)->tabla);
	}

	log_info(logger_invisible,"Lissandra.c: ejecutarOperacion() - Mensaje recibido %s", input);

	usleep(vconfig.retardo()*1000);

	if (parsed->valido) {
		switch (parsed->keyword){
		case SELECT:
			sem_wait(&mutexPeticionesPorTabla);
			bufferTabla=parsed->argumentos.INSERT.nombreTabla;
			semt = list_find(semaforosPorTabla, buscar);
			if(semt==NULL){
				//TODO:log error
				//TODO:Devolver mensaje error
				break;
			}
			sem_getvalue(&semt->semaforoSelect, &valorSemaforo);
			sem_post(&mutexPeticionesPorTabla);
			if(valorSemaforo >= 1){
				retorno = selectAPI(*parsed);
			}else{
				encolar_peticion(string_from_format(parsed->argumentos.SELECT.nombreTabla), string_from_format(input));
				retorno.TipoDeMensaje = TEXTO_PLANO;
				retorno.Argumentos.TEXTO_PLANO.texto = string_from_format("compactando");
			}
			break;
		case INSERT:
			sem_wait(&mutexPeticionesPorTabla);
			bufferTabla=parsed->argumentos.INSERT.nombreTabla;
			semt = list_find(semaforosPorTabla, buscar);
			if(semt==NULL){
				//TODO:log error
				//TODO:Devolver mensaje error
				break;
			}
			sem_getvalue(&semt->semaforoGral, &valorSemaforo);
			sem_post(&mutexPeticionesPorTabla);

			if(valorSemaforo >= 1){
				retorno = insertAPI(*parsed);
			}else{
				encolar_peticion(string_from_format(parsed->argumentos.INSERT.nombreTabla), string_from_format(input));
				retorno.TipoDeMensaje = TEXTO_PLANO;
				retorno.Argumentos.TEXTO_PLANO.texto = string_from_format("compactando");
			}
			break;
		case CREATE:
			retorno = createAPI(*parsed);
			break;
		case DESCRIBE:
			retorno = describeAPI(*parsed);
			break;
		case DROP:
			sem_wait(&mutexPeticionesPorTabla);
			bufferTabla=parsed->argumentos.DROP.nombreTabla;
			semt = list_find(semaforosPorTabla, buscar);
			if(semt==NULL){
				//TODO:log error
				//TODO:Devolver mensaje error
				break;
			}
			sem_getvalue(&semt->semaforoGral, &valorSemaforo);
			sem_post(&mutexPeticionesPorTabla);

			if(valorSemaforo >= 1){
				retorno = dropAPI(*parsed);
			}else{
				encolar_peticion(string_from_format(parsed->argumentos.DROP.nombreTabla), string_from_format(input));
				retorno.TipoDeMensaje = TEXTO_PLANO;
				retorno.Argumentos.TEXTO_PLANO.texto = string_from_format("compactando");
			}
			break;
		case RUN:
			liberarBloque(atoi(parsed->argumentos.RUN.path));
			//compactar(parsed->argumentos.RUN.path);
			break;
		case JOURNAL:
			getBloqueLibre();
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
	retorno.Argumentos.ERROR.mensajeError=string_from_format("Error en la recepcion del resultado.");
	mostrarRetorno(retorno);

	return retorno;
}

void encolar_peticion(char *tabla, char* peticion){
	if(dictionary_has_key(dPeticionesPorTabla, tabla)){
		t_list *encoladas = dictionary_get(dPeticionesPorTabla, tabla);
		list_add(encoladas, peticion);
	}else{
		t_list *encoladas = list_create();
		list_add(encoladas, peticion);
		dictionary_put(dPeticionesPorTabla, tabla, encoladas);
	}
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
		dictionary_iterator(memtable, dumpTabla);
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

void dumpTabla(char* nombreTable, void* value){
	t_list* list = (t_list*) value;

	if(list==NULL || list_size(list)==0){
		//TODO:log
		return;
	}
	char* path = string_from_format("%sTables/%s", config.punto_montaje, nombreTable);

	int numeroDump = cuentaArchivos(path);

	char* pathArchivo = string_from_format("%s/D%d.tmp", path, numeroDump);

	FILE* file = fopen(pathArchivo,"w");

	void dumpearTodosLosRegistros(void* reg){
		Registro* registro = (Registro*) reg;
		dumpRegistro(file, registro);
	}

	list_iterate(list, dumpearTodosLosRegistros);

	fclose(file);
	free(pathArchivo);
	free(path);

	void eliminarRegistro(void* elem){
		free(((Registro*)elem)->value);
		free(((Registro*)elem));
	}

	list_clean_and_destroy_elements(list, eliminarRegistro);
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
	char** bloques = string_get_string_as_array(listaDeBloques); //REVISION: se libera esto al final de la funcion

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
	string_iterate_lines(bloques, (void*)free);
	if(bloques)free(bloques);
	free(linea);
	return reg;
}
/*FIN FSEEK*/

void rutinas_de_finalizacion(){
	printf(BLU"\n█▀▀▀ █▀▀█ █▀▄▀█ █▀▀ 　 █▀▀█ █▀▀ 　 ▀▀█▀▀ █░░█ █▀▀█ █▀▀ █▀▀█ █▀▀▄ █▀▀ \n█░▀█ █▄▄█ █░▀░█ █▀▀ 　 █░░█ █▀▀ 　 ░░█░░ █▀▀█ █▄▄▀ █▀▀ █▄▄█ █░░█ ▀▀█ \n▀▀▀▀ ▀░░▀ ▀░░░▀ ▀▀▀ 　 ▀▀▀▀ ▀░░ 　 ░░▀░░ ▀░░▀ ▀░▀▀ ▀▀▀ ▀░░▀ ▀▀▀░ ▀▀▀ \n\n"STD);
	log_info(logger_invisible, "=============Finalizando LFS=============");
	fflush(stdout);

	/*Libero recursos*/
	config_destroy(configFile);
	dictionary_destroy(memtable);
	dictionary_destroy(dPeticionesPorTabla);
	bitarray_destroy(bitarray);
	close(miSocket);
	munmap(bitmap,metadataFS.blocks);
	log_destroy(logger_visible);
	log_destroy(logger_invisible);
	log_destroy(logger_error);

}
