/*
 * Gossiping.c
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */


#include "Gossiping.h"

int iniciar_gossiping() {


	char * ip_port_compresed = string_from_format("%s:%s", fconfig.ip, fconfig.puerto);
//	char * ip_port_compresed =

	//char * ip_port_compresed = string_from_format("%s:%s", fconfig.ip, fconfig.puerto);
	printf("IP propia : %s\n", ip_port_compresed);
	quitarCaracteresPpioFin(fconfig.ip_seeds);
	IPs = string_split(fconfig.ip_seeds, ",");

	quitarCaracteresPpioFin(fconfig.puerto_seeds);
	IPsPorts = string_split(fconfig.puerto_seeds, ",");
	listaMemoriasConocidas = list_create();

	//Reservo espacio para un knownMemory_t
	knownMemory_t * mem = malloc(sizeof(knownMemory_t)/*sizeof(int)+ sizeof(ip_port_compresed)+1*/ ) ;
			//Asigno a sus atributos los valores correspondientes
			mem->memory_number = atoi(fconfig.numero_memoria);
			mem->ip = string_from_format(fconfig.ip);
			mem->ip_port = string_from_format(fconfig.puerto);

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
	//liberarIPs(IPs);
	//liberarIPs(IPsPorts);
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
	 char * envio = NULL;
	 knownMemory_t * recupero;
	 printf("Armo paquete\n");
	 for(int i = 0; list_size(listaMemoriasConocidas ) > i ; i++) {
		 printf("Entro en lista\n");
		 recupero  = (knownMemory_t *)list_get(listaMemoriasConocidas , i);

		 concatenar_memoria(&envio, string_from_format("%d", recupero->memory_number) ,recupero->ip , recupero->ip_port);
	 }
		//concatenar_memoria(&envio,fconfig.numero_memoria  ,fconfig.ip , fconfig.puerto);

			printf("Mensaje corrido: %s \n",envio);
		request.TipoDeMensaje = GOSSIPING_REQUEST;
		printf("Paquete armado\n");
	 request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido = envio;

	 send_msg(socketSEEDS,request);
	 printf("Envio %d\n",request.TipoDeMensaje);
	 request = recv_msg(socketSEEDS);
	 printf("Respuesta\n");



	 char **descompresion = descomprimir_memoria(request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		printf("Mensaje corrido recibido: %s \n",request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	 	for(int i=0; descompresion[i]!=NULL; i+=3){
	 		knownMemory_t *memoria;
	 		if((memoria = machearMemoria(atoi(descompresion[i]))) == NULL){
	 			knownMemory_t *memoria = malloc(sizeof(knownMemory_t));
	 			memoria->memory_number = atoi(descompresion[i]);
	 			memoria->ip = string_from_format(descompresion[i+1]);
		 		memoria->ip_port = string_from_format(descompresion[i+2]);
	 			list_add(listaMemoriasConocidas, memoria);
	 		}


	 	}

	 	destruir_split_memorias(descompresion);

	 	printf("Fin GOSSIP\n");

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


Operacion recibir_gossiping (Operacion resultado){
	knownMemory_t * recupero;
	char * envio = NULL;
	printf("ENTRO FUNCION RECIBIR GOSSIPING\n");



/*

	char **descompresion = descomprimir_memoria(resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	for(int i=0; descompresion[i]!=NULL; i+=3){
		knownMemory_t *memoria;
		if((memoria = machearMemoria(atoi(descompresion[i]))) == NULL){
			printf("NO MACHEA\n");
			knownMemory_t *memoria = malloc(sizeof(knownMemory_t));
			memoria->memory_number = atoi(descompresion[i]);
			memoria->ip = descompresion[i+1];
			memoria->ip_port = descompresion[i+2];
			list_add(listaMemoriasConocidas, memoria);
		}
		printf("AGREGO EN LISTA\n %s\n%s\n%s\n",descompresion[i],descompresion[i+1],descompresion[i+2]);
		//memoria->memory_number = atoi(descompresion[i]);
		printf("MODIFIQUE NUMERO\n");
		//memoria->ip = descompresion[i+1];
		//memoria->ip_port = descompresion[i+2];
		printf("YA MODIFICO VALORES LISTA\n");
	}
	destruir_split_memorias(descompresion);
	*/
	// Ya agregue las memorias que me llegaron
	// Logica para enviar mi lista
	if(resultado.TipoDeMensaje == GOSSIPING_REQUEST){	// si es gossping request, proceso las memorias que me envian
	t_list *aux = list_create();
		 list_add_all(aux,listaMemoriasConocidas);
		 char **descompresion = descomprimir_memoria(resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		for(int i=0; descompresion[i]!=NULL; i+=3){
			knownMemory_t *memoria;
			if((memoria = machearMemoria(atoi(descompresion[i]))) == NULL){
				printf("NO MACHEA\n");


				knownMemory_t *memoria = malloc(sizeof(knownMemory_t));
				memoria->memory_number = atoi(descompresion[i]);
				memoria->ip = string_from_format(descompresion[i+1]);
				memoria->ip_port =string_from_format( descompresion[i+2]);
				printf("AGREGO EN LISTA\n %s\n%s\n%s\n",descompresion[i],descompresion[i+1],descompresion[i+2]);
				list_add(aux, memoria);
			} else {
				printf("MACHEA\n");
				printf("AGREGO EN LISTA\n %s\n%s\n%s\n",descompresion[i],descompresion[i+1],descompresion[i+2]);


			}



			printf("MODIFIQUE NUMERO\n");
			printf("YA MODIFICO VALORES LISTA\n");
		}
		destruir_split_memorias(descompresion);
		list_destroy(listaMemoriasConocidas); //Libero las referencias de la lista, sin liberar cada uno de sus elementos. Es decir, libero solo los nodos
		listaMemoriasConocidas = list_duplicate(aux); //Duplico la lista auxiliar con todos los elementos del nuevo describe, manteniendo los del anterior describe (son sus respecrtivos atributos de criterios), y eliminando los viejos (ya que nunca se agregaron a la listaAuxiliar)
			list_destroy(aux);
		// Ya agregue las memorias que me llegaron
		// Logica para enviar mi lista
		printf("Lista actualizada\n");
	}


	// Preparo mensaje para enviar mis memorias conocidas
	for(int i = 0; list_size(listaMemoriasConocidas ) > i ; i++) {
			 printf("Entro en lista\n");
			 recupero  = (knownMemory_t *)list_get(listaMemoriasConocidas , i);

			 concatenar_memoria(&envio, string_from_format("%d", recupero->memory_number) ,recupero->ip , recupero->ip_port);
			 printf("CONCATENO MENSAJE : %s\n",envio);
		 }
	resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido = envio;






	return resultado;
}


static knownMemory_t *machearMemoria(int numeroMemoria){
	bool buscar(void *elemento){
		return numeroMemoria == ((knownMemory_t*)elemento)->memory_number;
	}
	return (knownMemory_t*)list_find(listaMemoriasConocidas, buscar);
}
