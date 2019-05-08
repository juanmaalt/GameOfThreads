#include "cliente.h"

int connect_to_server(char * ip, char *port){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, port, &hints, &serverInfo);

	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int result = connect(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	if(result != 0){
		close(listenningSocket);
		fprintf(stderr, "cliente.c: conect_to_server: fallo la conexion con el servidor\n");
		return EXIT_FAILURE;
	}
	return listenningSocket;
}

