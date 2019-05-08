#include "server.h"

int enable_server(char *iPAddress, char* port){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	hints.ai_flags = *iPAddress;		// AI_PASSIVE -> Asigna el address del localhost: 127.0.0.1

	getaddrinfo(NULL, port, &hints, &serverInfo);

	int socketfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int result = bind(socketfd,serverInfo->ai_addr, serverInfo->ai_addrlen);

	freeaddrinfo(serverInfo); //Haya salido bien o mal el bind, ya no necesitamos serverInfo

	if(result != 0){
		close(socketfd);
		fprintf(stderr, "server.c: enable_server: puede que haya otro proceso escuchando el mismo puerto.\n");
		return EXIT_FAILURE;
	}

	return socketfd;
}



void *waiting_connections(int socket){
	//int socket = (int)socketVar;

	fd_set readfds; //Aca va a estar el (los) socket que necesite leer (que le enviemos algo) (que estan travados en un connect o un recv)
	fd_set masterfds;
	int maxSocketLen, newSocket, result;
	struct sockaddr_in direccionCliente;
	socklen_t clienteLen = sizeof(direccionCliente);

	FD_ZERO(&masterfds);
	FD_ZERO(&readfds);

	listen(socket, 5);

	FD_SET(socket, &masterfds);
	maxSocketLen = socket;

	for(;;){
		readfds = masterfds;
		result = select(maxSocketLen+1, &readfds, NULL, NULL, NULL); //FUNCION BLOQUEANTE QUE ESPERA QUE PASE ALGO

		if(result < 0){
			close(socket);
			fprintf(stderr, "server.c: waiting_conections: ocurrio un error inesperado");
			continue;
		}
		if(result == 0){
			printf("server.c: waiting_conections: no hubo actividad en ningun socket");
			continue; //Se saltea lo que queda del ciclo y vuelve a empezar. Este error probablemente no ocurra nunca
		}

		for(int unSocket=0; unSocket <= maxSocketLen; ++unSocket){ //Nos fijamos en cada uno de los sockets si hay actividad
			if (FD_ISSET(unSocket, &readfds)){
				if(unSocket == socket){
					newSocket = accept(socket, (struct sockaddr *) &direccionCliente, &clienteLen);
					if(newSocket == -1){
						printf("server.c: waiting_conections: error en aceptar nuevo cliente\n");
					}else{
						FD_SET(newSocket, &masterfds);
						if(newSocket > maxSocketLen)
							maxSocketLen = newSocket;
						//printf("server.c: waiting_conections: un cliente se ha conectado\n");
					}//fin if(newSocket == -1) else
				}else{ //Aca se define como responde el server ante un cliente que le quiere mandar un mensaje
					//result = serve_client(unSocket);
					if(result == EXIT_FAILURE){
						printf("server.c: waiting_conections: se ha desconectado un cliente\n");
						close(unSocket);
						FD_CLR(unSocket, &masterfds);
						continue;
					}
					//sem_post(&waiting);//seria el signal
					//send_msg(unSocket, get, "hola", NULL);
				}//fin if(unSocket == socket) else
			}//fin if(FD_ISSET(unSocket, &readfds))else
		}//fin for de unSocket
	}//FIN FOR INFINITO

	return NULL;
}
