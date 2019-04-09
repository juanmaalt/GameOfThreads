/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"


int main(void) {
	t_config* config = leer_config();

		/*Comando *res = parsear_linea("JOURNAL");

		printf("Palabra especial: %s\n", res->palabraEspecial);
		printf("Nombre tabla: %s\n", res->nombreTabla);
		printf("Key: %s\n", res->key);
		printf("Value: %s\n", res->value);
		printf("Timestamp: %s\n", res->timestamp);
		printf("Tipo consistencia: %s\n", res->tipoConsistencia);
		printf("Numero de particiones: %s\n", res->numeroParticiones);
		printf("Tiempo de compactacion: %s\n", res->compactacionTime);
		printf("Numero de memoria: %s\n", res->numeroMemoria);
		printf("Criterio: %s\n", res->criterio);
		printf("Path: %s\n", res->path);
		printf("Error en el parseo: %d\n", res->hayError);
		printf("Descripcion error: %s\n", res->descripcionError);

		free_comando(res);*/

	return 0;

}

t_config* leer_config() {
	return config_create("Kernel.config");
}
