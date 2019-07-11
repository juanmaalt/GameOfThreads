/*
 * Gossiping.c
 *
 *  Created on: 8 jul. 2019
 *      Author: facusalerno
 */

#include "Gossiping.h"

static void *main_gossiping(void *null);
static void hacer_gossiping(void *memoria);
static void modo_de_recuperacion(void);





int iniciar_gossiping(){
	if(USAR_SOLO_MEMORIA_PRINCIPAL)
		return EXIT_SUCCESS;
	if(pthread_create(&gossiping, NULL, main_gossiping, NULL) != 0)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}





static void *main_gossiping(void *null){
	for(;;){
		if(list_is_empty(memoriasExistentes))
			modo_de_recuperacion();
		else
			list_iterate(memoriasExistentes, hacer_gossiping);
		sleep(10);
	}
	return NULL;
}





static void hacer_gossiping(void *memoria){
	int socket = connect_to_server(((Memoria*)memoria)->ip, ((Memoria*)memoria)->puerto);
	if(socket == EXIT_FAILURE){
		remover_memoria((Memoria*)memoria);
		return;
	}

	Operacion op;
	t_list *seedsDeLaMemoria;

	op.TipoDeMensaje = GOSSIPING_REQUEST_KERNEL;
	send_msg(socket, op);
	op = recv_msg(socket);
	if(op.TipoDeMensaje == GOSSIPING_REQUEST)
		if(op.Argumentos.GOSSIPING_REQUEST.resultado_comprimido != NULL)
			seedsDeLaMemoria = procesar_gossiping(op.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	agregar_sin_repetidos(memoriasExistentes, seedsDeLaMemoria);
	list_destroy(seedsDeLaMemoria);

	/*void mostarLista(void * memoria){
		printf("%s %s %d \n",((Memoria*)memoria)->ip,((Memoria*)memoria)->puerto,((Memoria*)memoria)->numero);
	}
	printf(GRN"Estado de la lista despues de gossiping con una memoria del pool\n"STD);
	list_iterate(memoriasExistentes,mostarLista);
	printf(GRN"Fin g\n"STD);*/
}





static void modo_de_recuperacion(){
	do{
		int memoriaPrincipal = connect_to_server(fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
		if(memoriaPrincipal == EXIT_FAILURE){
			log_info(logger_visible, YEL"Modo de recuperacion: todas las memorias estan caidas. Esperando a la memoria principal %s:%s"STD, fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
			log_info(logger_invisible, "Modo de recuperacion: todas las memorias estan caidas. Esperando a la memoria principal %s:%s", fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
			sleep(3);
		}else{
			log_info(logger_visible, GRN"Conectado con la memoria principal %s:%s"STD, fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
			log_info(logger_invisible, "Conectado con la memoria principal %s:%s", fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
			Operacion ping;
			t_list *listaConLaMemoriaPrincipal; //La idea es formalizar el agregado de la memoria principal a la lista de memorias existentes
			ping.TipoDeMensaje = GOSSIPING_REQUEST_KERNEL;
			send_msg(memoriaPrincipal, ping);
			ping = recv_msg(memoriaPrincipal);
			listaConLaMemoriaPrincipal = procesar_gossiping(ping.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
			agregar_sin_repetidos(memoriasExistentes, listaConLaMemoriaPrincipal);
			list_destroy(listaConLaMemoriaPrincipal);
			return;
		}
	}while(1);
}



