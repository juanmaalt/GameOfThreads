#include "serializacion.h"

int send_msg(int socket, Operacion operacion) {
	size_t total=0;
	void* content=NULL;
	int longCadena = 0;
	Comando comando; //solo se usa para un chequeo en case COMANDO
	switch (operacion.TipoDeMensaje) {

	case TEXTO_PLANO:
		longCadena = strlen(operacion.Argumentos.TEXTO_PLANO.texto);
		total = sizeof(int) + sizeof(int) + sizeof(char) * longCadena; //Enum + cantidad caracteres cadena + cadena +  opCode
		content = malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		memcpy(content+sizeof(int), &longCadena, sizeof(int));
		memcpy(content+2*sizeof(int), operacion.Argumentos.TEXTO_PLANO.texto, sizeof(char)*(longCadena));
		break;

	case COMANDO://Nota: no puedo hacerlo en un paso tipo funcional
		comando = parsear_comando(operacion.Argumentos.COMANDO.comandoParseable);
		if (comando_validar(comando) == EXIT_FAILURE) {
			printf(RED"serializacion.c: send_command: el comando no es parseable"STD"\n");
			destruir_comando(comando);
			return EXIT_FAILURE;
		}
		destruir_comando(comando);
		longCadena = strlen(operacion.Argumentos.COMANDO.comandoParseable);
		total = sizeof(int) + sizeof(int) + sizeof(char) * longCadena;
		content = malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		memcpy(content+sizeof(int), &longCadena, sizeof(int));
		memcpy(content+2*sizeof(int),operacion.Argumentos.COMANDO.comandoParseable, sizeof(char)*longCadena);
		break;

	case REGISTRO:
		longCadena = strlen(operacion.Argumentos.REGISTRO.value);
		size_t tamValue = sizeof(char) * longCadena;
		total = sizeof(int) + sizeof(timestamp_t) + sizeof(uint16_t) + sizeof(int) + tamValue;
		content = malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		memcpy(content+sizeof(int), &(operacion.Argumentos.REGISTRO.timestamp), sizeof(timestamp_t));
		memcpy(content+sizeof(int)+sizeof(timestamp_t), &(operacion.Argumentos.REGISTRO.key), sizeof(uint16_t));
		memcpy(content+sizeof(int)+sizeof(timestamp_t) + sizeof(uint16_t), &longCadena, sizeof(int));
		memcpy(content+sizeof(int)+sizeof(timestamp_t) + sizeof(uint16_t) + sizeof(int), operacion.Argumentos.REGISTRO.value, tamValue);
		break;

	case ERROR:
		longCadena = strlen(operacion.Argumentos.ERROR.mensajeError);
		total = sizeof(int) + sizeof(int) + sizeof(char) * longCadena;
		content = malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		memcpy(content+sizeof(int), &longCadena, sizeof(int));
		memcpy(content+2*sizeof(int), operacion.Argumentos.ERROR.mensajeError, sizeof(char)*longCadena);
		break;

	case ERROR_JOURNAL:
		total = sizeof(int);
		content=malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		break;
	case ERROR_MEMORIAFULL:
		total = sizeof(int);
		content=malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		break;
	case GOSSIPING_REQUEST:
		longCadena = strlen(operacion.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		total = sizeof(int) + sizeof(int) +sizeof(char) * longCadena;
		content = malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		memcpy(content+sizeof(int), &longCadena, sizeof(int));
		memcpy(content+2*sizeof(int), operacion.Argumentos.GOSSIPING_REQUEST.resultado_comprimido, sizeof(char)*longCadena);
		break;
	case GOSSIPING_REQUEST_KERNEL:
		total = sizeof(int);
		content=malloc(total);
		memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
		break;
	case DESCRIBE_REQUEST:
		if(operacion.Argumentos.DESCRIBE_REQUEST.resultado_comprimido != NULL){
			longCadena = strlen(operacion.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
			total = sizeof(int) + sizeof(bool) + sizeof(int) + sizeof(char) * longCadena;
			content = malloc(total);
			memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
			memcpy(content+sizeof(int), &(operacion.Argumentos.DESCRIBE_REQUEST.esGlobal), sizeof(bool));
			memcpy(content+sizeof(int)+sizeof(bool), &longCadena, sizeof(int));
			memcpy(content+2*sizeof(int)+sizeof(bool), operacion.Argumentos.DESCRIBE_REQUEST.resultado_comprimido, sizeof(char)*longCadena);
		}else{
			longCadena = -1;
			total = sizeof(int) + sizeof(bool) + sizeof(int);
			content = malloc(total);
			memcpy(content, &(operacion.TipoDeMensaje), sizeof(int));
			memcpy(content+sizeof(int), &(operacion.Argumentos.DESCRIBE_REQUEST.esGlobal), sizeof(bool));
			memcpy(content+sizeof(int)+sizeof(bool), &longCadena, sizeof(int));
		}
		break;
	default:
		return EXIT_FAILURE;
	}

	int result = send(socket, content, total, 0);
	if (content != NULL)
			free(content);
	if (result <= 0) {
		printf(RED"serializacion.c: send_command: no se pudo enviar el mensaje"STD"\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

Operacion recv_msg(int socket) {
	Operacion retorno;
	int longitud = 0;
	int result = recv(socket, &(retorno.TipoDeMensaje), sizeof(int), 0);
	if (result <= 0)
		RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion");

	switch (retorno.TipoDeMensaje) {
	case TEXTO_PLANO:
		if(recv(socket, &longitud, sizeof(int), 0) <= 0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.TEXTO_PLANO.texto = calloc(longitud+1, sizeof(char));
		if(recv(socket, retorno.Argumentos.TEXTO_PLANO.texto, sizeof(char) * longitud, 0) <= 0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.TEXTO_PLANO.texto[longitud]='\0';
		break;
	case COMANDO:
		if(recv(socket, &longitud, sizeof(int), 0)<= 0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.COMANDO.comandoParseable = calloc(longitud+1,sizeof(char));
		if(recv(socket, retorno.Argumentos.COMANDO.comandoParseable,sizeof(char) * longitud, 0)<= 0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.COMANDO.comandoParseable[longitud]='\0';
		break;
	case REGISTRO:
		if(recv(socket, &(retorno.Argumentos.REGISTRO.timestamp), sizeof(timestamp_t), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		if(recv(socket, &(retorno.Argumentos.REGISTRO.key), sizeof(uint16_t), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		if(recv(socket, &longitud, sizeof(int), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.REGISTRO.value = calloc(longitud+1, sizeof(char));
		if(recv(socket, retorno.Argumentos.REGISTRO.value, sizeof(char) * longitud,0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.REGISTRO.value[longitud]='\0';
		break;
	case ERROR:
		if(recv(socket, &longitud, sizeof(int), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.ERROR.mensajeError = calloc(longitud+1, sizeof(char));
		if(recv(socket, retorno.Argumentos.ERROR.mensajeError, sizeof(char) * longitud, 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.ERROR.mensajeError[longitud]='\0';
		break;
	case ERROR_JOURNAL:
		break;
	case ERROR_MEMORIAFULL:
		break;
	case GOSSIPING_REQUEST:
		if(recv(socket, &longitud, sizeof(int), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.GOSSIPING_REQUEST.resultado_comprimido = calloc(longitud+1, sizeof(char));
		if(recv(socket, retorno.Argumentos.GOSSIPING_REQUEST.resultado_comprimido, sizeof(char)*longitud, 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		retorno.Argumentos.GOSSIPING_REQUEST.resultado_comprimido[longitud] = '\0';
		break;
	case GOSSIPING_REQUEST_KERNEL:
		break;
	case DESCRIBE_REQUEST:
		if(recv(socket, &retorno.Argumentos.DESCRIBE_REQUEST.esGlobal, sizeof(bool), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		if(recv(socket, &longitud, sizeof(int), 0)<=0)
			RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
		if(longitud == -1){
			retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido = NULL;
		}else{
			retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido = calloc(longitud+1, sizeof(char));
			if(recv(socket, retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido, sizeof(char)*longitud, 0)<=0)
				RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Es posible que se haya perdido la conexion")
			retorno.Argumentos.DESCRIBE_REQUEST.resultado_comprimido[longitud] = '\0';
		}
		break;
	default:
		RECV_FAIL("(Serializacion.c: recv_msg) Error en la recepcion del resultado. Tipo de operacion desconocido");
	}
	return retorno;
}

void destruir_operacion(Operacion op) {
	switch (op.TipoDeMensaje) {
	case COMANDO:
		if (op.Argumentos.COMANDO.comandoParseable != NULL)
			free(op.Argumentos.COMANDO.comandoParseable);
		return;
	case TEXTO_PLANO:
		if (op.Argumentos.TEXTO_PLANO.texto != NULL)
			free(op.Argumentos.TEXTO_PLANO.texto);
		return;
	case REGISTRO:
		if (op.Argumentos.REGISTRO.value != NULL)
			free(op.Argumentos.REGISTRO.value);
		return;
	case ERROR:
		if (op.Argumentos.ERROR.mensajeError != NULL)
			free(op.Argumentos.ERROR.mensajeError);
		return;
	case GOSSIPING_REQUEST:
		if(op.Argumentos.GOSSIPING_REQUEST.resultado_comprimido != NULL)
			free(op.Argumentos.GOSSIPING_REQUEST.resultado_comprimido);
		return;
	case DESCRIBE_REQUEST:
		if(op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido != NULL)
			free(op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
		return;
	}

}
