#include "serializacion.h"

int send_comando(int socket, Comando parsed){
	int keyword = parsed.keyword;
	char* arg1;
	char* arg2;
	char* arg3;
	char* arg4;
	int longArg1, longArg2, longArg3, longArg4;
	size_t total;
	void *content;
	//falta enviar longitudes antes de argumentos, y codear la funcion recv
    switch(keyword){
        case SELECT:
        	arg1 = parsed.argumentos.SELECT.nombreTabla;
            longArg1 = strlen(parsed.argumentos.SELECT.nombreTabla);
            arg2 = parsed.argumentos.SELECT.key;
            longArg2 = strlen(parsed.argumentos.SELECT.key);
            arg3 = NULL;
            longArg3 = 0;
            arg4 = NULL;
            longArg4 = 0;
            total = sizeof(int) + sizeof(int) + sizeof(char)*(longArg1) + sizeof(int) + sizeof(char)*(longArg2);
            content = malloc(total);
            //Keyword
            memcpy(content, &keyword, sizeof(int));
            //longArg1
            memcpy(content+sizeof(int), &longArg1, sizeof(int));
            //arg1
            memcpy(content+sizeof(int)+sizeof(int), arg1, sizeof(char)*longArg1);
            //longArg2
            memcpy(content+sizeof(int)+sizeof(int)+sizeof(char)*longArg1, &longArg2, sizeof(int));
            //arg2
            memcpy(content+sizeof(int)+sizeof(int)+sizeof(char)*longArg1+sizeof(int), arg2, sizeof(char)*longArg2);
            break;
        case INSERT:
        	arg1 = parsed.argumentos.INSERT.nombreTabla;
            longArg1 = strlen(parsed.argumentos.INSERT.nombreTabla);
            arg2 = parsed.argumentos.INSERT.key;
            longArg2 = strlen(parsed.argumentos.INSERT.key);
            arg3 = parsed.argumentos.INSERT.value;
            longArg3 = strlen(parsed.argumentos.INSERT.value);
            arg4 = parsed.argumentos.INSERT.value;
            longArg4 = strlen(parsed.argumentos.INSERT.value);
            total = sizeof(int) + sizeof(char)*(longArg1) + sizeof(char)*(longArg2) + sizeof(char)*(longArg3) + sizeof(char)*(longArg4);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int)); //Keyword
            memcpy(content+sizeof(int), arg1, sizeof(char)*longArg1);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1, arg2, sizeof(char)*longArg2);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1+sizeof(char)*longArg2, arg3, sizeof(char)*longArg3);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1+sizeof(char)*longArg2+sizeof(char)*longArg3, arg4, sizeof(char)*longArg4);
            break;
        case CREATE:
        	arg1 = parsed.argumentos.CREATE.nombreTabla;
            longArg1 = strlen(parsed.argumentos.CREATE.nombreTabla);
            arg2 = parsed.argumentos.CREATE.tipoConsistencia;
            longArg2 = strlen(parsed.argumentos.CREATE.tipoConsistencia);
            arg3 = parsed.argumentos.CREATE.numeroParticiones;
            longArg3 = strlen(parsed.argumentos.CREATE.numeroParticiones);
            arg4 = parsed.argumentos.CREATE.compactacionTime;
            longArg4 = strlen(parsed.argumentos.CREATE.compactacionTime);
            total = sizeof(int) + sizeof(char)*(longArg1) + sizeof(char)*(longArg2) + sizeof(char)*(longArg3) + sizeof(char)*(longArg4);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int));
            memcpy(content+sizeof(int), arg1, sizeof(char)*longArg1);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1, arg2, sizeof(char)*longArg2);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1+sizeof(char)*longArg2, arg3, sizeof(char)*longArg3);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1+sizeof(char)*longArg2+sizeof(char)*longArg3, arg4, sizeof(char)*longArg4);
            break;
        case DESCRIBE:
        	arg1 = parsed.argumentos.DESCRIBE.nombreTabla;
            longArg1 = strlen(parsed.argumentos.DESCRIBE.nombreTabla);
            arg2 = NULL;
            longArg2 = 0;
            arg3 = NULL;
            longArg3 = 0;
            arg4 = NULL;
            longArg4 = 0;
            total = sizeof(int) + sizeof(char)*(longArg1);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int));
            memcpy(content+sizeof(int), arg1, sizeof(char)*longArg1);
            break;
        case DROP:
        	arg1 = parsed.argumentos.DROP.nombreTabla;
            longArg1 = strlen(parsed.argumentos.DROP.nombreTabla);
            arg2 = NULL;
            longArg2 = 0;
            arg3 = NULL;
            longArg3 = 0;
            arg4 = NULL;
            longArg4 = 0;
            total = sizeof(int) + sizeof(char)*(longArg1);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int));
            memcpy(content+sizeof(int), arg1, sizeof(char)*longArg1);
            break;
        case JOURNAL:
        	arg1 = NULL;
            longArg1 = 0;
            arg2 = NULL;
            longArg2 = 0;
            arg3 = NULL;
            longArg3 = 0;
            arg4 = NULL;
            longArg4 = 0;
            total = sizeof(int);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int));
            break;
        case ADDMEMORY:
        	arg1 = parsed.argumentos.ADDMEMORY.numero;
            longArg1 = strlen(parsed.argumentos.ADDMEMORY.numero);
            arg2 = parsed.argumentos.ADDMEMORY.criterio;
            longArg2 = strlen(parsed.argumentos.ADDMEMORY.criterio);
            arg3 = NULL;
            longArg3 = 0;
            arg4 = NULL;
            longArg4 = 0;
            total = sizeof(int) + sizeof(char)*(longArg1) + sizeof(char)*(longArg2);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int));
            memcpy(content+sizeof(int), arg1, sizeof(char)*longArg1);
            memcpy(content+sizeof(int)+sizeof(char)*longArg1, arg2, sizeof(char)*longArg2);
            break;
        case RUN:
        	arg1 = parsed.argumentos.RUN.path;
            longArg1 = strlen(parsed.argumentos.RUN.path);
            arg2 = NULL;
            longArg2 = 0;
            arg3 = NULL;
            longArg3 = 0;
            arg4 = NULL;
            longArg4 = 0;
            total = sizeof(int) + sizeof(char)*(longArg1);
            content = malloc(total);
            memcpy(content, &keyword, sizeof(int));
            memcpy(content+sizeof(int), arg1, sizeof(char)*longArg1);
            break;
        default:
            return 0;
    }

	int result = send(socket, content, total, 0);
	if(result <= 0){
			printf("serializacion.c: send_msg: no se pudo enviar el mensaje\n");
			return EXIT_FAILURE;
		}


	return EXIT_SUCCESS;
}



Comando *recv_comando(int socket){
	Comando parsed;

	int result = recv(socket, &parsed.keyword, sizeof(int), 0);
	if(result <= 0){
			printf("serializacion.c: send_msg: no se pudo enviar el mensaje\n");
			return NULL;
		}

	char* arg1 = NULL;
	char* arg2 = NULL;
	char* arg3 = NULL;
	char* arg4 = NULL;
	int longArg1, longArg2, longArg3, longArg4;

    switch(parsed.keyword){
        case SELECT:
        	recv(socket, &longArg1, sizeof(int), 0);
        	arg1 = calloc(longArg1, sizeof(char));
        	recv(socket, arg1, sizeof(char)*longArg1, 0);

        	recv(socket, &longArg2, sizeof(int), 0);
        	arg2 = calloc(longArg2, sizeof(char));
        	recv(socket, arg2, sizeof(char)*longArg2, 0);

        	parsed.argumentos.SELECT.nombreTabla = arg1;
        	parsed.argumentos.SELECT.key = arg2;
            break;
        case INSERT:

            break;
        case CREATE:

            break;
        case DESCRIBE:

            break;
        case DROP:

            break;
        case JOURNAL:

            break;
        case ADDMEMORY:

            break;
        case RUN:

            break;
        default:
            return 0;
    }
    mostrar(parsed);
    while(1);
	return NULL;
}



void exit_error(int socket, char *msg, void *buffer){
	if (buffer != NULL) {
	       free(buffer);
	   }
	close(socket);
	printf("%s\n", msg);
	exit(1);
}
