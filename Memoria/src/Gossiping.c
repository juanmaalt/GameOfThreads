/*
 * Gossiping.c
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */


#include "Gossiping.h"

int iniciar_gossiping() {
	char * ip_port_compresed = comprimir_direccion(fconfig.ip , fconfig.puerto);
	//char * ip_port_compresed = string_from_format("%s:%s", fconfig.ip, fconfig.puerto);
	printf("IP propia : %s\n", ip_port_compresed);
	quitarCaracteresPpioFin(fconfig.ip_seeds);
	IPs = string_split(fconfig.ip_seeds, ",");

	quitarCaracteresPpioFin(fconfig.puerto_seeds);
	IPsPorts = string_split(fconfig.puerto_seeds, ",");
	listaMemoriasConocidas = list_create();

	//Reservo espacio para un MCB
	knownMemory_t * mem = malloc(sizeof(int)+ sizeof(ip_port_compresed)+1 ) ;
			//Asigno a sus atributos los valores correspondientes
			mem->memory_number = fconfig.numero_memoria;
			mem->ipandport = ip_port_compresed;

			list_add(listaMemoriasConocidas, (knownMemory_t *) mem);
	for (int i = 0; IPs[i] != NULL; ++i)	//Muestro por pantalla las IP seeds
		printf("IP %d: %s:%s:\n", i, IPs[i], IPsPorts[i]);

	if (pthread_create(&idGossipSend, NULL, conectar_seeds, NULL)) {
		printf(
				RED"Memoria.c: iniciar_gossiping: fallo la creacion hilo gossiping envios"STD"\n");
		printf("Error hilo\n");
		return EXIT_FAILURE;
	}

	printf("TERMINO\n");

	//if(pthread_create(&idConsola, NULL, recibir_comandos, NULL)){
	/*if (pthread_create(&idGossipReciv, NULL, recibir_seeds, NULL)) {
	 printf(
	 RED"Memoria.c: iniciar_gossiping: fallo la creacion hilo gossiping escucha"STD"\n");
	 return EXIT_FAILURE;
	 }
	 */

	return EXIT_SUCCESS;
}

void *conectar_seeds(void *null) { // hilo envia a las seeds
	pthread_detach(pthread_self());
	conectarConSeed();
	// puertoSocket = ConsultoPorMemoriasConocidas(puertoSocket);
	liberarIPs(IPs);
	liberarIPs(IPsPorts);
	//for (;;) {
	// Envia mensaje a las seeds que conoce
// }
	return NULL;
}

void *recibir_seeds(void *null) { // hilo que responde con las memorias conocidas
	//pthread_detach(pthread_self());

	for (;;) {
		//Escucha todo el tiempo y cuando llegue mensaje devuelve las memorias que conoce y estan activas
	}
	return NULL;
}
void quitarCaracteresPpioFin(char* cadena) {
	//char * temporal = malloc(sizeof(char) * (strlen(cadena) - 1)); //Me sobran 2 de comillas (-2) y +1 para el '\0'
	int i;

	for (i = 0; i < strlen(cadena) + 1; ++i) {
		cadena[i] = cadena[i + 1];
	}
	cadena[strlen(cadena) - 1] = '\0';

}

void liberarIPs(char** IPs) {
	if (IPs != NULL) {
		for (int i = 0; IPs[i] != NULL; ++i)
			free(*(IPs + i));
		free(IPs);
	}
}

void conectarConSeed() {
	// Se conecta con la seed para hacer el gossiping
	int conteo_seeds = 0; //Static
	printf("CONECTARCONSEED\n");
	for (; IPs[conteo_seeds] != NULL; conteo_seeds++) {
		printf("FOR IPS\n");
		int socket = connect_to_server(IPs[conteo_seeds],IPsPorts[conteo_seeds]);
		if (socket == EXIT_FAILURE) {
			log_error(logger_invisible, "La memoria no esta activa");
			printf("No activa\n");

			// return EXIT_FAILURE;
			// Debo quitar del diccionario esta memoria ya que no esta
		} else {
		 printf ("memoria activa\n");
		 log_error(logger_invisible, "Memoria conocida. Envia rmensaje");

		 ConsultoPorMemoriasConocidas(socket); //

		 }


	}
}


 void ConsultoPorMemoriasConocidas(int socketSEEDS) {
	 Operacion request;

	 request.TipoDeMensaje = GOSSIPING_REQUEST;
	 request.Argumentos.GOSSIPING_REQUEST.ipypuerto = list_get (listaMemoriasConocidas,0 );
	 send_msg(socketSEEDS,request);


 //char *tamanio = recv_msg(socketSEEDS, &tipo);
/*
 if (tipo.TipoDeMensaje == COMANDO)
 printf("Consulta de memorias conocidas falló. No se recibió respuesta.\n");
 if (tipo.TipoDeMensaje == TEXTO_PLANO)
 printf("Consulta exitosa. Se recibieron las memorias: %d\n",
 *tamanio);
*/
 //return *tamanio;
 }

