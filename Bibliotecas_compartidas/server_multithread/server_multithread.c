#include "server_multithread.h"


int threadConnection(int socket, void *funcionThread){
    int client_sock , *new_sock;
	struct sockaddr_in direccionCliente;
	socklen_t clienteLen = sizeof(direccionCliente);

	listen(socket , 5); //TODO: establecer cantidad en funcion de cantidad de ips que tengo en config +1 el kernel

	printf("server_multithread.c: in threadConnection\n");

	while((client_sock = accept(socket, (struct sockaddr *)&direccionCliente, &clienteLen)) != -1){
        printf("server_multithread.c: connection accepted\n");

        pthread_t sniffer_thread;
	    new_sock = malloc(sizeof(int));
	    *new_sock = client_sock;

	    if( pthread_create( &sniffer_thread , NULL ,  funcionThread , (void*) new_sock) < 0){
            perror(RED"server_multithread.c: no se pudo crear el hilo"STD"\n");
            return EXIT_FAILURE;
	    }


	    // hago un pthread join para evitar que los siguientes hilos pisen el proceso del hilo actual
	    //pthread_join( sniffer_thread , NULL);
	    //printf("server_multithread.c: handler assigned\n");
	}

	if (client_sock < 0){
	        perror(RED"server_multithread.c: accept failed"STD"\n");
	        return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}




