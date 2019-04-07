#include "serializacion.h"

int send_msg(int socket, int id, char *clave, char *valor){
	int lenClave=0, lenValor=0;

	if(clave == NULL){
		clave = (void *) calloc(5, sizeof(char));
		strcpy(clave, "NULL"); //Para no hacer mucho quilombo simulamos que es null
		clave[4] = '\0';
	}

	if(valor == NULL){
		valor = (void *) calloc(5, sizeof(char));
		strcpy(valor, "NULL"); //Para no hacer mucho quilombo simulamos que es null
		valor[4] = '\0';
	}

	lenClave = strlen(clave);
	lenValor = strlen(valor);

	void *content = malloc(sizeof(int)*3 + sizeof(char)*(lenClave) + sizeof(char)*(lenValor+1));

	memcpy(content, &id, sizeof(int)); //Se copa el id en el primer sector de memoria
	memcpy(content+sizeof(int), &lenClave, sizeof(int)); //lenClave en el segundo sector de memoria
	memcpy(content+2*sizeof(int), &lenValor, sizeof(int)); //lenValor en el tercer sector de memoria
	strcpy(content+3*sizeof(int), clave); //la clave en el anteultimo sector de memoria
	strcpy(content+3*sizeof(int) + sizeof(char)*(lenClave), valor); //El valor en el ultimo sector de memoria

	int result = send(socket, content, sizeof(int)*3 + sizeof(char)*(lenClave) + sizeof(char)*(lenValor), 0);
	if(result <= 0){
		free(clave);
		free(content);
		free(valor);
		printf("serializacion.c: send_msg: no se pudo enviar el mensaje\n");
		return EXIT_FAILURE;
	}
	free(content);
	free(clave);
	free(valor);

	return EXIT_SUCCESS;
}



Instruccion *recv_msg(int socket){
	Instruccion *contenido = malloc(sizeof(Instruccion));
	int result, lenClave=0, lenValor=0;
	contenido->id = (void *) calloc(1, sizeof(int));
	result = recv(socket, contenido->id, sizeof(int), 0);	if(result <= 0){printf("serializacion.c: recv_msg: no se pudo recibir el id del mensaje, posiblemente alguien esta desconectado\n"); free(contenido->id); *contenido->id=error_de_ejecucion; return contenido;}
	result = recv(socket, &lenClave, sizeof(int), 0);	if(result <= 0){printf("serializacion.c: recv_msg: no se pudo recibir el lenClave del mensaje\n");*contenido->id=error_de_ejecucion; return contenido;}
	result = recv(socket, &lenValor, sizeof(int), 0);	if(result <= 0){printf("serializacion.c: recv_msg: no se pudo recibir el lenValor del mensaje\n");*contenido->id=error_de_ejecucion; return contenido;}

	contenido->clave = (void *) calloc(lenClave+1, sizeof(char)); //El +1 es para el \0
	result = recv(socket, contenido->clave, sizeof(char)*lenClave, 0); //Aca recibimos lo que queda del mensaje que es la longitud del mismo
	if(result <= 0){
		free(contenido->id);
		free(contenido->clave);
		printf("serializacion.c: recv_msg: no se pudo recibir el mensaje\n");
		return (Instruccion *)EXIT_FAILURE;
	}
	contenido->clave[sizeof(char)*lenClave]='\0';

	contenido->valor = (void *) calloc(lenValor+1, sizeof(char));
	result = recv(socket, contenido->valor, sizeof(char)*lenValor, 0);
	if(result <= 0){
		free(contenido->clave);
		free(contenido->valor);
		free(contenido);
		printf("serializacion.c: recv_msg: no se pudo recibir el mensaje\n");
		return (Instruccion *)EXIT_FAILURE;
	}
	contenido->valor[sizeof(char)*lenValor]='\0';

	return contenido;
}



void exit_error(int socket, char *msg, void *buffer){
	if (buffer != NULL) {
	       free(buffer);
	   }
	close(socket);
	printf("%s\n", msg);
	exit(1);
}


int send_msg_test(int socket){
	int id;
	printf("Defina el ID del mensaje con un numero\n3:get\n4:set\n5:store\n");
	scanf("%d", &id);
	switch(id){
		case get:
			id=get;
			break;
		case set:
			id=set;
			break;
		case store:
			id=store;
			break;
		default: ;
	}

	char *clave = prepare_msg();
	char *valor = prepare_msg();

	int lenClave = strlen(clave);
	int lenValor = strlen(valor);

	void *content = malloc(sizeof(int)*3 + sizeof(char)*(lenClave) + sizeof(char)*(lenValor+1)); //Si se agrega +1 a lenClave, del otro lado llega un string vacio. Si no se agrega un +1 a lenValor, valgrind detecta un error de escritura en strcpy

	memcpy(content, &id, sizeof(int)); //Se copa el id en el primer sector de memoria
	memcpy(content+sizeof(int), &lenClave, sizeof(int)); //lenClave en el segundo sector de memoria
	memcpy(content+2*sizeof(int), &lenValor, sizeof(int)); //lenValor en el tercer sector de memoria
	strcpy(content+3*sizeof(int), clave); //la clave en el anteultimo sector de memoria
	strcpy(content+3*sizeof(int) + sizeof(char)*(lenClave), valor); //El valor en el ultimo sector de memoria


	int result = send(socket, content, sizeof(int)*3 + sizeof(char)*(lenClave) + sizeof(char)*(lenValor), 0);
	if(result <= 0){
		printf("serializacion.c: send_msg_test: no se pudo enviar el mensaje\n");
		return EXIT_FAILURE;
	}

	printf("mensaje enviado!: id: %d, lenClave: %d, lenValoi: %d, clave: %s, valor: %s\n", id, lenClave, lenValor, clave, valor);

	free(content);
	free(clave);
	free(valor);
	return EXIT_SUCCESS;
}



char *prepare_msg(void){
	char *buffer= readline("Ingrese un mensaje: ");
	return buffer; //Recordar hacer free de esto en la funcion send_msg()
}



int recv_msg_test(int socket){
	int id=0, lenClave=0, lenValor=0;

	int result;
	result = recv(socket, &id, sizeof(int), 0);			if(result <= 0){printf("serializacion.c: recv_msg_test: no se pudo recibir el id del mensaje\n");return EXIT_FAILURE;}
	result = recv(socket, &lenClave, sizeof(int), 0);	if(result <= 0){printf("serializacion.c: recv_msg_test: no se pudo recibir el lenClave del mensaje\n");return EXIT_FAILURE;}
	result = recv(socket, &lenValor, sizeof(int), 0);	if(result <= 0){printf("serializacion.c: recv_msg_test: no se pudo recibir el lenValor del mensaje\n");return EXIT_FAILURE;}

	char *clave = (void *) calloc(lenClave+1, sizeof(char)); //El ++1 es para el \0
	result = recv(socket, clave, sizeof(char)*lenClave, 0); //Aca recibimos lo que queda del mensaje que es la longitud del mismo
	if(result <= 0){
		printf("serializacion.c: recv_msg_test: no se pudo recibir el mensaje\n");
		return EXIT_FAILURE;
	}
	clave[sizeof(char)*lenClave]='\0';

	char *valor = (void *) calloc(lenValor+1, sizeof(char));
	result = recv(socket, valor, sizeof(char)*lenValor, 0);
	if(result <= 0){
			printf("serializacion.c: recv_msg_test: no se pudo recibir el mensaje\n");
			return EXIT_FAILURE;
		}
	valor[sizeof(char)*lenValor]='\0';

	switch(id){
		case get:
			printf("Mensaje recibido!: id: get, lenClave: %d, clave: %s\n", lenClave, clave);
			break;
		case set:
			printf("Mensaje recibido!: id: set, lenClave: %d, lenValor: %d, clave: %s, valor: %s\n", lenClave, lenValor, clave, valor);
			break;
		case store:
			printf("Mensaje recibido!: id: store, lenClave: %d, clave: %s\n", lenClave, clave);
			break;
		default: ;
		}

	free(clave);
	free(valor);

	return EXIT_SUCCESS;
}


