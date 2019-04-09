/*
 * parser.c
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 */
#include "parser.h"

Comando *parsear_linea(char* cad){
	Comando *resultado = (Comando *) malloc(sizeof(Comando));
	char *cadena = (char *) malloc((sizeof(char)*strlen(cad))+1);
	strcpy(cadena, cad);
	strcat(cadena, " "); //Este es un fix bien a lo cabeza, se necesita minimo un espacio al final de cada linea que se parsee por algunos casos excepcionales, por ejemplo INSERT tablaA 3 valueEjemplo

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

char *obtener_palabra_especial(char *cadena){
	int offset=0;
	char *base = cadena;
	cadena += seek; //Aca seek deberia ser 0 asi que no hace nada

	if(*cadena == '\0') //Para el no trivial caso en que me ingresen algo del estilo: "INSERT tablaA 3 valueEjemplo". Insert espera un timestamp, pero este podria llegar a ser null. Por otro lado el la cadena va a estar siempre apuntando a seek+1 donde ese +1 siempre va a representar el espacio siguiente, excepto en este caso donde va a representar el \0
		return NULL;

	for( ; *cadena != ' '; ++cadena, ++offset) ;
	cadena = base;

	char *resultado = (char *) malloc((sizeof(char)*offset)+1);
	strncpy(resultado, cadena+seek, sizeof(char)*offset);
	resultado[sizeof(char)*offset] = '\0';
	seek += offset+1;

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
	char *base = cadena;
	cadena += seek;

	if(*cadena == '\0')
		return NULL;

	for( ; *cadena != ' '; ++cadena, ++offset) ;
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

