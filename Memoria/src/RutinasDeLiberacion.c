/*
 * RutinasDeLiberacion.c
 *
 *  Created on: 15 jun. 2019
 *      Author: fdalmaup
 */

#include "RutinasDeLiberacion.h"

void liberarMCB(void* MCBAdestruir) {
	if ((MCB_t *) MCBAdestruir != NULL)
		free(MCBAdestruir);
}

void liberarRegistroTablaPags(void* registroAdestruir) {
	MCB_t* MCBLibre;

	bool compararConMCB(void* MCBAComparar){
		if((((registroTablaPag_t *) registroAdestruir)->nroMarco) == ((MCB_t *)MCBAComparar)->nroMarco){
			return true;
		}else{
			return false;
		}
	}

	t_list* listaConMCBBuscado = list_filter(memoriaPrincipal.listaAdminMarcos,compararConMCB);

	if (list_is_empty(listaConMCBBuscado)) {
			list_destroy(listaConMCBBuscado);
			//VER SI HAY ALGO PARA RETORNAR
	}
	MCBLibre= (MCB_t*) list_remove(listaConMCBBuscado, 0);

	list_destroy(listaConMCBBuscado);

	queue_push(memoriaPrincipal.marcosLibres, (MCB_t*)MCBLibre);

	if ((registroTablaPag_t *) registroAdestruir != NULL)
		free(registroAdestruir);
}

void liberarSegmento(void* segmentoAdestruir) {
	if (((segmento_t *) segmentoAdestruir)->pathTabla != NULL)
		free(((segmento_t *) segmentoAdestruir)->pathTabla);

	list_destroy_and_destroy_elements(((segmento_t *) segmentoAdestruir)->tablaPaginas->registrosPag, liberarRegistroTablaPags);

	if (((segmento_t *) segmentoAdestruir)->tablaPaginas != NULL)
		free(((segmento_t *) segmentoAdestruir)->tablaPaginas);

	if ((segmento_t *) segmentoAdestruir != NULL)
		free(segmentoAdestruir);
}

void liberarRecursos(void) {
	log_info(logger_visible,"Finalizando proceso Memoria...");
	if (memoriaPrincipal.memoria != NULL)
		free(memoriaPrincipal.memoria);

	if(list_is_empty(tablaSegmentos.listaSegmentos)){
		list_destroy(tablaSegmentos.listaSegmentos);
	}else{
		list_destroy_and_destroy_elements(tablaSegmentos.listaSegmentos, liberarSegmento);
	}

	queue_clean(memoriaPrincipal.marcosLibres);
	queue_destroy(memoriaPrincipal.marcosLibres);

	list_destroy_and_destroy_elements(memoriaPrincipal.listaAdminMarcos,liberarMCB);


	if (pathLFS != NULL)
		free(pathLFS);

	config_destroy(configFile);
	log_destroy(logger_invisible);
	log_destroy(logger_visible);
}
