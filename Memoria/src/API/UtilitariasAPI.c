/*
 * UtilitariasAPI.c
 *
 *  Created on: 23 jun. 2019
 *      Author: fdalmaup
 */

#include "UtilitariasAPI.h"

//TODO: Usar find en lugar de filter (mas performante)

bool verificarExistenciaSegmento(char* nombreTabla,
		segmento_t ** segmentoAVerificar) {
	char* pathSegmentoBuscado = malloc(
			sizeof(char) * (strlen(nombreTabla) + strlen(pathLFS)) + 1);

	strcpy(pathSegmentoBuscado, pathLFS);
	strcat(pathSegmentoBuscado, nombreTabla);

	//Recorro tabla de segmentos buscando pathSegmentoBuscado

	bool segmentoCoincidePath(void* comparado) {
		//printf(RED"pathSegmentoBuscado: %s\npathComparado: %s"STD"\n",pathSegmentoBuscado,obtenerPath((segmento_t*)comparado));
		if (strcmp(pathSegmentoBuscado, obtenerPath((segmento_t*) comparado))) {
			return false;
		}
		return true;
	}

	t_list* listaConSegmentoBuscado = list_filter(tablaSegmentos.listaSegmentos,
			segmentoCoincidePath);

	//Para ver que el path es el correcto

	//printf("El path del segmento buscado es: %s\n", pathSegmentoBuscado);
	free(pathSegmentoBuscado);

	if (list_is_empty(listaConSegmentoBuscado)) {
		list_destroy(listaConSegmentoBuscado);
		log_info(logger_invisible,"UtilitariasAPI: verificarExistenciaSegmento: no se encontro el segmento, se procede a crear segmento de tabla");
		return false;
	}
	//Tomo de la lista filtrada el segmento con el path coincidente

	*segmentoAVerificar = (segmento_t*) list_get(listaConSegmentoBuscado, 0);

	//Elimino la lista filtrada
	list_destroy(listaConSegmentoBuscado);

	return true;
}

//Busca en cada registro de la tabla de paginas el indice de marco y me fijo si coincide la key

//En vez de pasarle value le paso la operación que quiero hacer con value (modificarlo con insert o tomarlo con select)

bool contieneKey(segmento_t* segmentoElegido, uint16_t keyBuscada,
		registroTablaPag_t ** registroResultado) {
	//1. Tomo la tabla de paginas del segmento

	t_list * regsDelSegmentoElegido =
			segmentoElegido->tablaPaginas->registrosPag;

	//2. Por cada registro de la tabla, me meto en la memoria y tomo la key
	//3. En cada marco me fijo si la key que tiene == keyBuscada
	//3.1 En caso de que la key sea la keyBuscada tomo el valor del value, y countUso++
	//3.2 Si la key NO es la buscada sigo con el siguiente marco

	bool compararConPagina(void* registroAComparar) {
		uint16_t *keyPagina = malloc(sizeof(uint16_t));

		void* direccionPagina = memoriaPrincipal.memoria
				+ memoriaPrincipal.tamanioMarco
						* (((registroTablaPag_t*) registroAComparar)->nroMarco);
		memcpy(keyPagina, direccionPagina + sizeof(timestamp_t),
				sizeof(uint16_t));

		if (*keyPagina == keyBuscada) {
			//Aumento el uso de la pagina
			free(keyPagina);
			return true;
		}
		free(keyPagina);
		return false;
	}

	t_list* listaConRegistroBuscado = list_filter(regsDelSegmentoElegido,
			compararConPagina);

	if (list_is_empty(listaConRegistroBuscado)) {
		list_destroy(listaConRegistroBuscado);
		return false;
	}

	*registroResultado = (registroTablaPag_t*) list_remove(
			listaConRegistroBuscado, 0);
	list_destroy(listaConRegistroBuscado);

	return true;

}



