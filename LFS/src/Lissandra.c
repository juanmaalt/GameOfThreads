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

void *connection_handler(void *nSocket){
    int socket = *(int*)nSocket;
    TipoDeMensaje tipo;
    char *resultado = recv_msg(socket, &tipo);

	if(resultado == NULL){
		return NULL; //Es importante realizar este chequeo devolviendo EXIT_FAILURE
	}

	printf("Hemos recibido algo!\n");
	if(tipo == COMANDO)
		ejecutarOperacion(resultado);
	if(tipo == TEXTO_PLANO){
		if(strcmp(resultado, "handshake")==0)
			handshakeMemoria(socket);
	}else
			printf("No se pudo conectar la Memoria\n");

	if(resultado != NULL)
		free(resultado);

	return NULL;
}

int main(void) {
	//Se hacen las configuraciones iniciales para log y config
	if(configuracion_inicial() == EXIT_FAILURE){
		printf(RED"Memoria.c: main: no se pudo generar la configuracion inicial"STD"\n");
		return EXIT_FAILURE;
	}
	ver_config();
	/*Meter funcion para levantar las variables de tiempo retardo y tiempo_dump*/

	memtable = inicializarMemtable();

	//Habilita el server y queda en modo en listen / * Inicializar la File System principal
	/*
	int miSocket = enable_server(config.ip, config.puerto_escucha);
	log_info(logger_invisible, "Servidor encendido, esperando conexiones");
	threadConnection(miSocket, connection_handler);
	*/

	printf("Antes de agregarDatos\n");
	agregarDatos(memtable);//funcion para pruebas

	//Inicio consola
	if(iniciar_consola() == EXIT_FAILURE){
			printf(RED"Memoria.c: main: no se pudo levantar la consola"STD"\n");
			return EXIT_FAILURE;
	}
	pthread_join(idConsola,NULL);


	config_destroy(configFile);
	return EXIT_SUCCESS;
}


/*INICIO FUNCIONES CONFIG*/
int configuracion_inicial(){
	logger_visible = iniciar_logger(true);
	if(logger_visible == NULL){
		printf(RED"Lissandra.c: configuracion_inicial: error en 'logger_visible = iniciar_logger(true);'"STD"\n");
		return EXIT_FAILURE;
	}

	logger_invisible = iniciar_logger(false);
	if(logger_visible == NULL){
		printf(RED"Lissandra.c: configuracion_inicial: error en 'logger_invisible = iniciar_logger(false);'"STD"\n");
		return EXIT_FAILURE;
	}

	configFile = leer_config();
	if(configFile == NULL){
		printf(RED"Lissandra.c: configuracion_inicial: error en el archivo 'LFS.config'"STD"\n");
		return EXIT_FAILURE;
	}
	extraer_data_config();

	return EXIT_SUCCESS;
}


t_log* iniciar_logger(bool visible) {
	return log_create("LFS.log", "LFS", visible, LOG_LEVEL_INFO);
}


t_config* leer_config(){
	return config_create("LFS.config");
}


void extraer_data_config() {
	config.ip = config_get_string_value(configFile, "IP");
	config.puerto_escucha = config_get_string_value(configFile, "PUERTO_ESCUCHA");
	config.punto_montaje = config_get_string_value(configFile, "PUNTO_MONTAJE");
	//config.retardo = config_get_string_value(configFile, "RETARDO");
	config.tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	//config.tiempo_dump = config_get_string_value(configFile, "TIEMPO_DUMP");
}


void ver_config(){
	log_info(logger_visible, "IP=%s", config.ip);
	log_info(logger_visible, "PUERTO_ESCUCHA=%s", config.puerto_escucha);
	log_info(logger_visible, "PUNTO_MONTAJE=%s", config.punto_montaje);
	//log_info(logger_visible, "RETARDO=%s", config.retardo);
	log_info(logger_visible, "TAMANIO_VALUE=%s", config.tamanio_value);
	//log_info(logger_visible, "TIEMPO_DUMP=%s", config.tiempo_dump);
}
/*FIN FUNCIONES CONFIG*/


t_dictionary* inicializarMemtable(){
	return dictionary_create();
}


void handshakeMemoria(int socketMemoria){
	printf("Se conectó la Memoria\n");
	char* tamanio = config.tamanio_value;

	send_msg(socketMemoria, TEXTO_PLANO, tamanio);
}

void agregarDatos(t_dictionary* memtable){
	Metadata_tabla* meta = malloc(sizeof(Metadata_tabla));

	meta->compaction_time=60000;
	meta->consistency="SC";
	meta->partitions=4;

	Registro* reg1 = malloc(sizeof(Registro));
	Registro* reg2 = malloc(sizeof(Registro));
	Registro* reg3 = malloc(sizeof(Registro));
	Registro* reg4 = malloc(sizeof(Registro));

	reg1->key=3;
	reg1->timestamp=1558492233084;
	reg1->value="pepe";

	reg2->key=4;
	reg2->timestamp=1558492233085;
	reg2->value="carlos";

	reg3->key=3;
	reg3->timestamp=1558492233086;
	reg3->value="pepe2";

	reg4->key=4;
	reg4->timestamp=1558492233087;
	reg4->value="carlos2";

	t_list* lista = list_create();
	char* tabla="tabla";

	dictionary_put(memtable, tabla, lista);//Agrego una tabla y su data;

	lista = dictionary_get(memtable, tabla);//obtengo la data, en el insert debería checkear que este dato no sea null

	list_add(lista,meta);
	list_add(lista,reg1);
	list_add(lista,reg2);
	list_add(lista,reg3);
	list_add(lista,reg4);

	printf("Fin de agregarDatos\n");

}


int iniciar_consola(){
	if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
		printf(RED"Memoria.c: iniciar_consola: fallo la creacion de la consola"STD"\n");
		return EXIT_FAILURE;
	}

	//No hay pthread_join. Alternativamente hay pthread_detach en la funcion recibir_comando. Hacen casi lo mismo
	return EXIT_SUCCESS;
}


int ejecutarOperacion(char* input){ //TODO: TIPO de retorno Resultado
	Comando *parsed = malloc(sizeof(Comando));
	*parsed = parsear_comando(input);
	//TODO: funciones pasandole userInput y parsed por si necesito enviar algo o usar algun dato parseado

	if(parsed->valido){
		switch(parsed->keyword){
			case SELECT:
				selectAPI(*parsed);
				break;
			case INSERT:
				insertAPI(*parsed);
				break;
			case CREATE:
			case DESCRIBE:
			case DROP:
				break;
			default:
				fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n", parsed->keyword);
		}

		destruir_operacion(*parsed);
	}else{
		fprintf(stderr, RED"La request no es valida"STD"\n");
	}
	return EXIT_SUCCESS; //MOMENTANEO
}

uint16_t obtenerKey(Registro* registro){
		return registro->key;
}

timestamp_t obtenerTimestamp(Registro* registro){
		return registro->timestamp;
}
