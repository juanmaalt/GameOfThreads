/*
 * Gossiping.c
 *
 *  Created on: 17 jun. 2019
 *      Author: utnso
 */

#include "Gossiping.h"

int iniciar_gossiping() {

	//pthread_mutex_init(&mutexGossiping, NULL);


//	char * ip_port_compresed =

//char * ip_port_compresed = string_from_format("%s:%s", fconfig.ip, fconfig.puerto);
//printf("IP propia : %s\n", ip_port_compresed);
	quitarCaracteresPpioFin(fconfig.ip_seeds);
	IPs = string_split(fconfig.ip_seeds, ",");

	quitarCaracteresPpioFin(fconfig.puerto_seeds);
	IPsPorts = string_split(fconfig.puerto_seeds, ",");
	listaMemoriasConocidas = list_create();

	//Reservo espacio para un knownMemory_t
	knownMemory_t * mem = malloc(
			sizeof(knownMemory_t)/*sizeof(int)+ sizeof(ip_port_compresed)+1*/);
	//Asigno a sus atributos los valores correspondientes
	mem->memory_number = atoi(fconfig.numero_memoria);
	mem->ip = string_from_format(fconfig.ip);
	mem->ip_port = string_from_format(fconfig.puerto);
	//pthread_mutex_lock(&mutexGossiping);
	list_add(listaMemoriasConocidas, (knownMemory_t *) mem);
	//pthread_mutex_unlock(&mutexGossiping);
	for (int i = 0; IPs[i] != NULL; ++i)	//Muestro por pantalla las IP seeds
		log_info(logger_gossiping,
				"GOSSIPING.C: iniciar_gossiping: IP SEED %d: %s:%s:", i, IPs[i],
				IPsPorts[i]);

	if (pthread_create(&idGossipSend, NULL, conectar_seeds, NULL)) {

		log_error(logger_error,
				"GOSSIPING.C: iniciar_gossiping: fallo la creacion hilo gossiping");
		return EXIT_FAILURE;
	}

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
	//conectarConSeed();
	// puertoSocket = ConsultoPorMemoriasConocidas(puertoSocket);
	//liberarIPs(IPs);
	//liberarIPs(IPsPorts);
	for (;;) {
		log_info(logger_invisible,
				"GOSSIPING.C: conectar_seeds: inicio gossiping");
		log_info(logger_gossiping,
				"GOSSIPING.C: conectar_seeds: inicio gossiping");
		conectarConSeed();
		usleep(vconfig.retardoGossiping * 1000);
		// Envia mensaje a las seeds que conoce
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
	//printf("CONECTARCONSEED\n");
	for (; IPs[conteo_seeds] != NULL; conteo_seeds++) {
		//printf("FOR IPS\n");
		int socket = connect_to_server(IPs[conteo_seeds],
				IPsPorts[conteo_seeds]);
		if(socket == EXIT_FAILURE)
			printf("ESTA DANDO ERROR\n");
		printf(YEL"IMPRIMO SOCKET %d\n IP PUERTO %s:%s"STD"\n",socket,IPs[conteo_seeds],IPsPorts[conteo_seeds]);
		if (socket == EXIT_FAILURE) {
			log_info(logger_visible,
					"GOSSIPING.C:conectarConSeed: La memoria seed no esta activa %s:%s",
					IPs[conteo_seeds], IPsPorts[conteo_seeds]);

			bool buscarMemoria(void * buscoMemoria) {
				//printf("Buscar Memoria %s %s\n",IPs[conteo_seeds],IPsPorts[conteo_seeds]);
				int comparoPort = strcmp(IPsPorts[conteo_seeds],
						((knownMemory_t*) buscoMemoria)->ip_port);
				int comparoIP = strcmp(IPs[conteo_seeds],
						((knownMemory_t*) buscoMemoria)->ip);
				//return ((IPs[conteo_seeds] == ((knownMemory_t*)buscoMemoria)->ip) && (IPsPorts[conteo_seeds] == ((knownMemory_t*)buscoMemoria)->ip_port));
				//printf("Buscar primeri %s  BUSCAR SEGUNDO %s\n",IPsPorts[conteo_seeds],((knownMemory_t*)buscoMemoria)->ip_port);

				if ((comparoPort * comparoPort + comparoIP * comparoIP) == 0) // Como me puede dar negativo, saco los modulos. En el caso de que la IP y el puerto sean iguales devuelve 0 la suma
					return 1; //Tengo que ir a destruirMemoria
				else
					return 0;	// No hago nada
			}
			void destruirMemoria(void *destruir) {
				free(((knownMemory_t*) destruir)->ip);
				free(((knownMemory_t*) destruir)->ip_port);
				free(((knownMemory_t*) destruir));
				//printf("Destrui Memoria\n");
				return;
			}
			//pthread_mutex_lock(&mutexGossiping);
			list_remove_and_destroy_by_condition(listaMemoriasConocidas,
					buscarMemoria, destruirMemoria);
			//printf("Sali Destruir\n");

			int sizeList = list_size(listaMemoriasConocidas);
			int indexList = 0;
			while ((sizeList > 0) && (indexList < sizeList)) {
				//printf(YEL"ENTRO AL WHILE\n"STD);
				knownMemory_t * memoriaLista;
				memoriaLista = (knownMemory_t *) list_get(
						listaMemoriasConocidas, indexList);
				int comparoPort = strcmp(fconfig.puerto,
						((knownMemory_t*) memoriaLista)->ip_port);
				int comparoIP = strcmp(fconfig.ip,
						((knownMemory_t*) memoriaLista)->ip);
				if ((comparoPort * comparoPort + comparoIP * comparoIP) == 0)
					indexList++;
				else {
					// TIRO SOCKET, SI DA ERROR, lo tengo que limpiar de la lista
					int socketLista = connect_to_server(
							((knownMemory_t*) memoriaLista)->ip,
							((knownMemory_t*) memoriaLista)->ip_port);
					if (socketLista == EXIT_FAILURE) {
						list_remove(listaMemoriasConocidas, indexList);
						sizeList = list_size(listaMemoriasConocidas);
						//printf(YEL"SEED CAIDA Y OTRA MEMORIA TAMBIEN\n"STD);
						indexList++;
					} else {
						close (socketLista);
					}
					//free(((knownMemory_t*)memoriaLista)->ip);
					//free(((knownMemory_t*)memoriaLista)->ip_port);
					//free(((knownMemory_t*)memoriaLista));

				}

			}
			//pthread_mutex_unlock(&mutexGossiping);
			// return EXIT_FAILURE;
			// Debo quitar del diccionario esta memoria ya que no esta
		} else {
			//printf ("memoria activa\n");
			log_info(logger_gossiping,
					"GOSSIPING.C:conectarConSeed: Memoria conocida. Enviar mensaje %s:%s ",
					IPs[conteo_seeds], IPsPorts[conteo_seeds]);

			ConsultoPorMemoriasConocidas(socket); //
			close(socket);
		}

	}
}

void ConsultoPorMemoriasConocidas(int socketSEEDS) {

	Operacion request;
	char * envio = NULL;
	knownMemory_t * recupero;
	//printf("Armo paquete\n");
	//pthread_mutex_lock(&mutexGossiping);
	for (int i = 0; list_size(listaMemoriasConocidas) > i; i++) {
		//printf("Entro en lista\n");
		recupero = (knownMemory_t *) list_get(listaMemoriasConocidas, i);

		concatenar_memoria(&envio,
				string_from_format("%d", recupero->memory_number), recupero->ip,
				recupero->ip_port);
	}
	//concatenar_memoria(&envio,fconfig.numero_memoria  ,fconfig.ip , fconfig.puerto);

	//printf("Mensaje corrido: %s \n",envio);
	request.TipoDeMensaje = GOSSIPING_REQUEST;
	//printf("Paquete armado\n");
	request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido = envio;
	log_info(logger_visible,
			"GOSSIPING.C:ConsultoPorMemoriasConocidas: Envio gossiping: %s",
			envio);
	send_msg(socketSEEDS, request);
	//pthread_mutex_unlock(&mutexGossiping);
	//printf("Envio %d\n",request.TipoDeMensaje);
	request = recv_msg(socketSEEDS);
	log_info(logger_visible,
			"GOSSIPING.C:ConsultoPorMemoriasConocidas: Respuesta gossiping: %s",
			request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	//printf("Respuesta\n");

	t_list *aux = list_create();
	/*
	 recibo lista.
	 elemento no machea => agrego en aux (es nuevo)
	 elemento machea => voy a la lista y la quito. agrego en aux (es viejo)
	 Al final los que quedan en la lista vieja son las bajas
	 */
	char **descompresion = descomprimir_memoria(
			request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	//printf("Mensaje corrido recibido: %s \n",request.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	//pthread_mutex_lock(&mutexGossiping);
	for (int i = 0; descompresion[i] != NULL; i += 3) {

		knownMemory_t *memoria;
		if ((memoria = machearMemoria(atoi(descompresion[i]))) == NULL) {
			//NO MACHEA, agrego a la lista aux
			knownMemory_t *memoria = malloc(sizeof(knownMemory_t));
			memoria->memory_number = atoi(descompresion[i]);
			memoria->ip = string_from_format(descompresion[i + 1]);
			memoria->ip_port = string_from_format(descompresion[i + 2]);
			list_add(aux, memoria);
		} else {
			// Las que machean, las saco de la lista de conocidas y lo agrego a la lista aux
			knownMemory_t *memoria = malloc(sizeof(knownMemory_t));
			memoria->memory_number = atoi(descompresion[i]);
			memoria->ip = string_from_format(descompresion[i + 1]);
			memoria->ip_port = string_from_format(descompresion[i + 2]);
			list_add(aux, memoria);
			/* bool buscarMemoria(void * buscoMemoria){
			 return atoi(descompresion[i]) == ((knownMemory_t*)buscoMemoria)->memory_number;
			 }
			 void destruirMemoria ( void * destruir){
			 free(destruir);
			 return;
			 }
			 list_remove_and_destroy_by_condition(listaMemoriasConocidas,buscarMemoria,destruirMemoria);*/

		}

	}

	destruir_split_memorias(descompresion);
	list_destroy(listaMemoriasConocidas); //Libero las referencias de la lista, sin liberar cada uno de sus elementos. Es decir, libero solo los nodos
	listaMemoriasConocidas = list_duplicate(aux); //Duplico la lista auxiliar con todos los elementos del nuevo describe, manteniendo los del anterior describe (son sus respecrtivos atributos de criterios), y eliminando los viejos (ya que nunca se agregaron a la listaAuxiliar)
	//pthread_mutex_unlock(&mutexGossiping);
	list_destroy(aux);

	log_info(logger_invisible,
			"GOSSIPING.C:ConsultoPorMemoriasConocidas:Fin GOSSIP");
	log_info(logger_visible,
			"GOSSIPING.C:ConsultoPorMemoriasConocidas:Fin GOSSIP");
	//close(socketSEEDS);

}

Operacion recibir_gossiping(Operacion resultado) {
	knownMemory_t * recupero;
	char * envio = NULL;
	log_info(logger_gossiping,
			"GOSSIPING.C:recibir_gossiping: ENTRO FUNCION RECIBIR GOSSIPING");
	//pthread_mutex_lock(&mutexGossiping);
	if (resultado.TipoDeMensaje == GOSSIPING_REQUEST) {	// si es gossping request, proceso las memorias que me envian
		t_list *aux = list_create();
		t_list *aux_filtro = list_create();
		//pthread_mutex_lock(&mutexGossiping);
		log_info(logger_gossiping,
				"GOSSIPING.C:recibir_gossiping: Mensaje recibido %s",resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		//log_info(logger_gossiping,
				//"GOSSIPING.C:recibir_gossiping: Recibo mensaje gossiping: %s",
				//resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		char **descompresion = descomprimir_memoria(
				resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		//pthread_mutex_lock(&mutexGossiping);
		list_add_all(aux, listaMemoriasConocidas);
		for (int i = 0; descompresion[i] != NULL; i += 3) {
			knownMemory_t *memoria;
			if ((memoria = machearMemoria(atoi(descompresion[i]))) == NULL) {
				//printf("NO MACHEA\n");

				knownMemory_t *memoria = malloc(sizeof(knownMemory_t));
				memoria->memory_number = atoi(descompresion[i]);
				memoria->ip = string_from_format(descompresion[i + 1]);
				memoria->ip_port = string_from_format(descompresion[i + 2]);
				//printf("PUERTO SOCKET %s:%s\n",descompresion[i + 1], descompresion[i + 2]);
				int socketNew = connect_to_server(descompresion[i + 1],
						descompresion[i + 2]);
				//printf("SOCKET NEW : %d\n",socketNew);
				if (socketNew == EXIT_FAILURE) {
					//printf("FALLO SOCKET\n");
					log_info(logger_gossiping,
							"GOSSIPING.C:recibir_gossiping: La memoria no esta activa %s:%s",
							descompresion[i + 1], descompresion[i + 2]);
					//printf("No activa\n");

					// Debo quitar de la lista esta memoria ya que no esta
					for (int j = 0; list_size(listaMemoriasConocidas) > j; j++) {
						//printf("Entro a filtar para quitar de lista\n");
						recupero = (knownMemory_t *) list_get(listaMemoriasConocidas, j);
						int cmpIP = strcmp(recupero->ip, descompresion[j + 1]);
						int cmpIPPORT = strcmp(recupero->ip_port,
								descompresion[j + 2]);
						if ((cmpIP * cmpIP + cmpIPPORT * cmpIPPORT) != 0)
							list_add(aux_filtro, recupero);
						concatenar_memoria(&envio,
								string_from_format("%d",
										recupero->memory_number), recupero->ip,
								recupero->ip_port);
						//printf("CONCATENO MENSAJE MEMORIA NO ACTIVA QUITAR : %s\n",envio);
					}
					list_destroy(aux); //Libero las referencias de la lista, sin liberar cada uno de sus elementos. Es decir, libero solo los nodos
					aux = list_duplicate(aux_filtro); //Duplico la lista auxiliar con todos los elementos del nuevo describe, manteniendo los del anterior describe (son sus respecrtivos atributos de criterios), y eliminando los viejos (ya que nunca se agregaron a la listaAuxiliar)
					list_destroy(aux_filtro);

				} else {
					list_add(aux, memoria);
					//printf("AGREGO EN LISTA cierro socket\n %s\n%s\n%s\n",descompresion[i],descompresion[i+1],descompresion[i+2]);
					close(socketNew);
					//printf("Cierro Socket\n");
				}
				//printf("AGREGO EN LISTA\n %s\n%s\n%s\n",descompresion[i],descompresion[i+1],descompresion[i+2]);
				//list_add(aux, memoria);
			} else {
				//printf("MACHEA\n");
				//printf("AGREGO EN LISTA\n %s\n%s\n%s\n",descompresion[i],descompresion[i+1],descompresion[i+2]);

			}

			//printf("MODIFIQUE NUMERO\n");
			//printf("YA MODIFICO VALORES LISTA\n");
		}
		envio = NULL;
		destruir_split_memorias(descompresion);
		list_destroy(listaMemoriasConocidas); //Libero las referencias de la lista, sin liberar cada uno de sus elementos. Es decir, libero solo los nodos
		listaMemoriasConocidas = list_duplicate(aux); //Duplico la lista auxiliar con todos los elementos del nuevo describe, manteniendo los del anterior describe (son sus respecrtivos atributos de criterios), y eliminando los viejos (ya que nunca se agregaron a la listaAuxiliar)
		//pthread_mutex_unlock(&mutexGossiping);
		list_destroy(aux);
		// Ya agregue las memorias que me llegaron
		// Logica para enviar mi lista
		log_info(logger_gossiping,
				"GOSSIPING.C:recibir_gossiping: Lista de memorias actualizada");
	}

	// Preparo mensaje para enviar mis memorias conocidas
	//pthread_mutex_lock(&mutexGossiping);
	for (int i = 0; list_size(listaMemoriasConocidas) > i; i++) {
		//printf("Entro en lista\n");
		recupero = (knownMemory_t *) list_get(listaMemoriasConocidas, i);

		concatenar_memoria(&envio,
				string_from_format("%d", recupero->memory_number), recupero->ip,
				recupero->ip_port);
		//printf("CONCATENO MENSAJE : %s\n",envio);
	}
	resultado.TipoDeMensaje = GOSSIPING_REQUEST;
	resultado.Argumentos.GOSSIPING_REQUEST.resultado_comprimido = envio;
	log_info(logger_visible,
			"GOSSIPING.C:recibir_gossiping: Envio mensaje gossiping %s", envio);
	//pthread_mutex_unlock(&mutexGossiping);

	return resultado;
}

static knownMemory_t *machearMemoria(int numeroMemoria) {
	bool buscar(void *elemento) {
		return numeroMemoria == ((knownMemory_t*) elemento)->memory_number;
	}
	return (knownMemory_t*) list_find(listaMemoriasConocidas, buscar);
}
