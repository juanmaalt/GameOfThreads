/*
 * Gossiping.c
 *
 *  Created on: 8 jul. 2019
 *      Author: facusalerno
 */

#include "Gossiping.h"

static void *main_gossiping(void *null);
static void hacer_gossiping(void *memoria);

int iniciar_gossiping(){
	if(USAR_SOLO_MEMORIA_PRINCIPAL)
		return EXIT_SUCCESS;
	if(pthread_create(&gossiping, NULL, main_gossiping, NULL) != 0)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static void *main_gossiping(void *null){
	for(;;){
		list_iterate(memoriasExistentes, hacer_gossiping);
		sleep(10);
	}
	return NULL;
}

static void hacer_gossiping(void *memoria){
	int socket = connect_to_server(((Memoria*)memoria)->ip, ((Memoria*)memoria)->puerto);
	if(socket == EXIT_FAILURE)
		return;

	Operacion op;
	t_list *memoriasActivas;
	op.TipoDeMensaje = GOSSIPING_REQUEST_KERNEL;
	send_msg(socket, op);
	op = recv_msg(socket);
	if(op.TipoDeMensaje == GOSSIPING_REQUEST)
		if(op.Argumentos.GOSSIPING_REQUEST.resultado_comprimido != NULL)
			memoriasActivas = procesar_gossiping(op.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
	agregar_sin_repetidos(memoriasExistentes, memoriasActivas);
	list_destroy(memoriasActivas);
}



