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
		printf(RED"Lissandra.c: main: no se pudo generar la configuracion inicial"STD"\n");
		return EXIT_FAILURE;
	}
	ver_config();
	//Meter funcion para levantar las variables de tiempo retardo y tiempo_dump


	//podria poner un getchar y pedir que se especifique si hay un directorio de fileSystem creado o no
	/*Inicio el File System*/
	checkEstructuraFS();

	/*Inicio la Memtable*/
	memtable = inicializarMemtable();

	agregarDatos(memtable);//funcion para pruebas TODO:Borrar esto

	/*Inicio la consola*/
	if(iniciar_consola() == EXIT_FAILURE){
		log_error(logger_invisible,	"Lissandra.c: main: no se pudo levantar la consola");
		return EXIT_FAILURE;
	}

	/*Habilita al File System como server y queda en modo en listen*/

	int miSocket = enable_server(config.ip, config.puerto_escucha);
	log_info(logger_invisible, "Servidor encendido, esperando conexiones");
	threadConnection(miSocket, connection_handler);

	/*Libero recursos*/
	config_destroy(configFile);
	dictionary_destroy(memtable);

	return EXIT_SUCCESS;
}

/*INICIO FUNCION PARA MANEJO DE HILOS*/
void *connection_handler(void *nSocket){
	pthread_detach(pthread_self());
	int socket = *(int*) nSocket;
	Operacion resultado;

	resultado = recv_msg(socket);

	printf("Hemos recibido algo!\n");

	switch (resultado.TipoDeMensaje){
	case COMANDO:
		//TODO: logear comando recibido
		printf("Comando recibido: %s\n",resultado.Argumentos.COMANDO.comandoParseable);
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
	config.retardo = config_get_string_value(configFile, "RETARDO");
	config.tamanio_value = config_get_string_value(configFile, "TAMANIO_VALUE");
	//config.tiempo_dump = config_get_string_value(configFile, "TIEMPO_DUMP");
}


void ver_config(){
	log_info(logger_visible, "IP=%s", config.ip);
	log_info(logger_visible, "PUERTO_ESCUCHA=%s", config.puerto_escucha);
	log_info(logger_visible, "PUNTO_MONTAJE=%s", config.punto_montaje);
	log_info(logger_visible, "RETARDO=%s", config.retardo);
	log_info(logger_visible, "TAMANIO_VALUE=%s", config.tamanio_value);
	//log_info(logger_visible, "TIEMPO_DUMP=%s", config.tiempo_dump);
}
/*FIN FUNCIONES CONFIG*/

/*INICIO FUNCIONES COMPLEMENTARIAS*/

t_dictionary* inicializarMemtable(){
	return dictionary_create();
}

void handshakeMemoria(int socketMemoria){
	printf("Se conectó la Memoria\n");

	Operacion handshake;
	handshake.TipoDeMensaje=TEXTO_PLANO;
	handshake.Argumentos.TEXTO_PLANO.texto=string_from_format(config.tamanio_value);

	/*Mando el tamaño del value*/
	send_msg(socketMemoria, handshake);

	/*Recibo un nuevo mensaje de la memoria*/
	destruir_operacion(handshake);
	handshake = recv_msg(socketMemoria);

	switch(handshake.TipoDeMensaje){
		case TEXTO_PLANO:
			if(strcmp(handshake.Argumentos.TEXTO_PLANO.texto, "handshake pathLFS")==0){
				destruir_operacion(handshake);
				handshake.TipoDeMensaje=TEXTO_PLANO;
				handshake.Argumentos.TEXTO_PLANO.texto=string_from_format(config.punto_montaje);
				send_msg(socketMemoria, handshake);
			}
			else{printf("No se pudo conectar la Memoria\n");}
			break;
		case ERROR:
		case COMANDO:
		case REGISTRO:
			break;
	}
}

int iniciar_consola(){
	if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
		log_error(logger_invisible, "Lissandra.c: iniciar_consola: fallo la creacion de la consola");
		return EXIT_FAILURE;
	}

	printf("en iniciar consola\n");

	//No hay pthread_join. Alternativamente hay pthread_detach en la funcion recibir_comando. Hacen casi lo mismo
	return EXIT_SUCCESS;
}


Operacion ejecutarOperacion(char* input){ //TODO: TIPO de retorno Resultado
	Comando *parsed = malloc(sizeof(Comando));
	Operacion retorno;
	*parsed = parsear_comando(input);

	usleep(atoi(config.retardo)*1000);

	if (parsed->valido) {
		switch (parsed->keyword) {
		case SELECT:
			retorno = selectAPI(*parsed);
			break;
		case INSERT:
			retorno = insertAPI(*parsed);
			break;
		case CREATE:
			retorno = createAPI(*parsed);
			break;
		case DESCRIBE:
		case DROP:
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

	return retorno;
}

uint16_t obtenerKey(Registro* registro){
		return registro->key;
}

timestamp_t obtenerTimestamp(Registro* registro){
		return registro->timestamp;
}

/*FIN FUNCIONES COMPLEMENTARIAS*/


/*INICIO FUNCIONES TEST*/
void agregarDatos(t_dictionary* memtable){
	Registro* reg1 = malloc(sizeof(Registro));
	Registro* reg2 = malloc(sizeof(Registro));
	Registro* reg3 = malloc(sizeof(Registro));
	Registro* reg4 = malloc(sizeof(Registro));

	reg1->key=3;
	reg1->timestamp=1558492233084;
	reg1->value=string_from_format("pepe");

	reg2->key=4;
	reg2->timestamp=1558492233085;
	reg2->value=string_from_format("carlos");

	reg3->key=3;
	reg3->timestamp=1558492233086;
	reg3->value=string_from_format("pepe2");

	reg4->key=4;
	reg4->timestamp=1558492233087;
	reg4->value=string_from_format("carlos2");

	t_list* lista = list_create();
	char* tabla=string_from_format("tabla");

	dictionary_put(memtable, tabla, lista);//Agrego una tabla y su data;

	lista = dictionary_get(memtable, tabla);//obtengo la data, en el insert debería checkear que este dato no sea null

	list_add(lista,reg1);
	list_add(lista,reg2);
	list_add(lista,reg3);
	list_add(lista,reg4);

}

/*FIN FUNCIONES TEST*/
