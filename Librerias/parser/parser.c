/*
 * parser.c
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 */
#include "parser.h"

Comando *parsear_linea(char* cad){
	Comando *resultado = (Comando *) malloc(sizeof(Comando));
	char *cadena = formatear_cadena(cad);

	resultado->hayError = 0; //Empezamos creyendo que no hay error hasta que pase lo contrario

	resultado->palabraEspecial = obtener_palabra_especial(cadena);

	if(strcmp(resultado->palabraEspecial, "SELECT") == 0){
		resultado->nombreTabla = extraer_palabra_y_guardar(cadena);
		resultado->key = extraer_palabra_y_guardar(cadena);
	}else

	if(strcmp(resultado->palabraEspecial, "INSERT") == 0){
		resultado->nombreTabla = extraer_palabra_y_guardar(cadena);
		resultado->key = extraer_palabra_y_guardar(cadena);
		resultado->value = extraer_palabra_y_guardar(cadena);
		resultado->timestamp = extraer_palabra_y_guardar(cadena);
	}else

	if(strcmp(resultado->palabraEspecial, "CREATE") == 0){
		resultado->nombreTabla = extraer_palabra_y_guardar(cadena);
		resultado->tipoConsistencia = extraer_palabra_y_guardar(cadena);
		resultado->numeroParticiones = extraer_palabra_y_guardar(cadena);
		resultado->compactacionTime = extraer_palabra_y_guardar(cadena);
	}else

	if(strcmp(resultado->palabraEspecial, "DESCRIBE") == 0){
		resultado->nombreTabla = extraer_palabra_y_guardar(cadena);
	}else

	if(strcmp(resultado->palabraEspecial, "DROP") == 0){
		resultado->nombreTabla = extraer_palabra_y_guardar(cadena);
	}else

	if(strcmp(resultado->palabraEspecial, "JOURNAL") == 0){
		//No deberia hacer nada
	}else

	if(strcmp(resultado->palabraEspecial, "ADD MEMORY") == 0){
		resultado->numeroMemoria = extraer_palabra_y_guardar(cadena);
		char *palabra = obtener_palabra_especial(cadena);
		if(palabra != NULL && strcmp(palabra, "TO") == 0){
			resultado->criterio = extraer_palabra_y_guardar(cadena);
		}else{
			resultado->hayError = 1;
			char *mensaje = "Error sintactico. Recuerde, ADD MEMORY [numero] TO [criterio]";
			resultado->descripcionError = (char *) malloc((sizeof(char)*strlen(mensaje))+1);
			strcpy(resultado->descripcionError, mensaje);
			resultado->descripcionError[sizeof(char)*strlen(mensaje)] = '\0';
		}
	}else

	if(strcmp(resultado->palabraEspecial, "RUN") == 0){
		resultado->path = extraer_palabra_y_guardar(cadena);
	}else{
		resultado->hayError = 1;
		char *mensaje = "Palabra especial invalida";
		resultado->descripcionError = (char *) malloc((sizeof(char)*strlen(mensaje))+1);
		strcpy(resultado->descripcionError, mensaje);
		resultado->descripcionError[sizeof(char)*strlen(mensaje)] = '\0';
	}
	free(cadena);
	return resultado;
}

char *formatear_cadena(char *cad){
	char *cadena = (char *) malloc((sizeof(char)*strlen(cad))+1);
	strcpy(cadena, cad);
	cadena[(sizeof(char)*strlen(cad))] = '\0';
	return cadena;
}

char *obtener_palabra_especial(char *cadena){
	int offset=0;
	char *base;
	cadena += seek; //Aca seek deberia ser 0 asi que no hace nada

	if(*cadena == '\0') //Siempre es bueno chequear esto
		return NULL;

	for( ; *cadena == ' '; ++cadena, ++seek) ; //Elimina los espacios en blanco, incrementando el punteor seek para que apunte al siguiente caracter
	base = cadena; //Actualizamos la base de la cadena, ya que la idea es omitir el espacio

	for( ; *cadena != ' ' && *cadena != '\0'; ++cadena, ++offset) ; //Preguntamos cual es la longituf (offset) de la siguiente palabra hasta un espacio o \0
	cadena = base; //una vez que tenemos la longitud, necesitamos volver a poner la cadena en su estado original, es decir, al principio de la ultima palabra encontrada

	char *resultado = (char *) malloc((sizeof(char)*offset)+1);
	strncpy(resultado, cadena+seek, sizeof(char)*offset);
	resultado[sizeof(char)*offset] = '\0';
	seek += offset+1; //Actualizamos el seek para que la proxima vez que se ejecute la funcion, salteemos esta palabra. El +1 es para que en el siguiente ciclo, seek ya este apuntando a lo que sigue, y no al ultmo caracter de este palabra

	if(strcmp(resultado, "ADD") == 0){
		char *palabra = obtener_palabra_especial(cadena);
		if(strcmp(palabra, "MEMORY") == 0){
			strcat(resultado, " ");
			strcat(resultado, palabra);
		}
	}

	return resultado;
}

char *extraer_palabra_y_guardar(char *cadena){
	int offset=0;
	char *base;
	cadena += seek;

	if(*cadena == '\0')
		return NULL;

	for( ; *cadena == ' '; ++cadena, ++seek) ;
	base = cadena;

	for( ; *cadena != ' ' && *cadena != '\0'; ++cadena, ++offset) ;
	cadena = base;

	char *resultado = (char *) malloc((sizeof(char)*offset)+1);
	strncpy(resultado, cadena+seek, sizeof(char)*offset);
	resultado[sizeof(char)*offset] = '\0';
	seek += offset+1;
	return resultado;
}

void free_comando(Comando *comando){
	if(comando->palabraEspecial != NULL)
		free(comando->palabraEspecial);
	if(comando->nombreTabla != NULL)
		free(comando->nombreTabla);
	if(comando->key != NULL)
		free(comando->key);
	if(comando->value != NULL)
		free(comando->value);
	if(comando->timestamp != NULL)
		free(comando->timestamp);
	if(comando->tipoConsistencia != NULL)
		free(comando->tipoConsistencia);
	if(comando->numeroParticiones != NULL)
		free(comando->numeroParticiones);
	if(comando->compactacionTime != NULL)
		free(comando->compactacionTime);
	if(comando->numeroMemoria != NULL)
		free(comando->numeroMemoria);
	if(comando->criterio != NULL)
		free(comando->criterio);
	if(comando->path != NULL)
		free(comando->path);
	if(comando->descripcionError != NULL)
		free(comando->descripcionError);
	if(comando != NULL)
		free(comando);
}

