/*
 * Semaforos.c
 *
 *  Created on: 31 jul. 2019
 *      Author: facusalerno
 */

#include "Semaforos.h"

int iniciar_semaforos(){
	/*Mutex generales*/
	sem_init(&mutexMemtable, 0, 1);
	sem_init(&mutexAgregarTablaMemtable, 0, 1);
	sem_init(&mutexVConfig, 0, 1);

	/*Compactacion*/
	semaforosPorTabla =list_create();
	sem_init(&mutexPeticionesPorTabla, 0, 1);

	/*Request activas*/
	requestActivas = list_create();
	sem_init(&mutexRequestActivas, 0, 1);
	return EXIT_SUCCESS;
}





/*Compactacion*/

void bloquearTabla(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_post(&mutexPeticionesPorTabla); //Ojo con este semaforo, se usa solo para acceder a la lista o alguna variable que no sea semaforo
	sem_wait(&(((SemaforoCompactacion*)semt)->semaforoGral)); //Bloqueamos nuevas request acceder
	/*El semaforo de la tabla deberia estar en 1. Este wait lo decrementa a 0 haciendo que ninguna
	 * nueva request pueda entrar*/
}





void bloquearSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	void *semt = list_find(semaforosPorTabla, buscar);
	sem_post(&mutexPeticionesPorTabla);
	sem_wait(&(((SemaforoCompactacion*)semt)->semaforoSelect));
	//log_info(logger_visible, "Compactar(%s): Se inicio la escritura de los bloques de la tabla %s", tabla, tabla);
	/*El semaforo de select de la tabla deberia estar en 1. Este wait lo decrementa a 0 haciendo que ninguna
	 * nueva request pueda hacer un select*/
}





void desbloquearTabla(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	SemaforoCompactacion *semt = (SemaforoCompactacion*) list_find(semaforosPorTabla, buscar);
	for(int i=0; i<=semt->peticionesEnEspera; ++i)
		sem_post(&semt->semaforoGral);
	/*El desbloqueo de la tabla consiste en hacer signal del semaforo que la bloqueaba antes, tantas veces como request
	 * haya esperando. Esto es por que durante la compactacion, podrian estar varias request en espera*/
	sem_post(&mutexPeticionesPorTabla);
}





void desbloquearSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	SemaforoCompactacion *semt = (SemaforoCompactacion*) list_find(semaforosPorTabla, buscar);
	for(int i=0; i<=semt->peticionesEnEsperaSelect; ++i)
		sem_post(&semt->semaforoSelect);
	sem_post(&mutexPeticionesPorTabla);
}





void tryExecute(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	SemaforoCompactacion *semt = (SemaforoCompactacion*) list_find(semaforosPorTabla, buscar);
	if(semt == NULL){
		sem_post(&mutexPeticionesPorTabla);
		return;
	}
	int valorSemaforo;
	sem_getvalue(&semt->semaforoGral, &valorSemaforo);
	if(valorSemaforo >= 1){
		sem_post(&mutexPeticionesPorTabla);
		return;
	}
	++semt->peticionesEnEspera;
	sem_post(&mutexPeticionesPorTabla);
	sem_wait(&semt->semaforoGral);
}





void tryExecuteSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	SemaforoCompactacion *semt = (SemaforoCompactacion*) list_find(semaforosPorTabla, buscar);
	if(semt == NULL){
		sem_post(&mutexPeticionesPorTabla);
		return;
	}
	int valorSemaforo;
	sem_getvalue(&semt->semaforoSelect, &valorSemaforo);
	if(valorSemaforo >= 1){
		sem_post(&mutexPeticionesPorTabla);
		return;
	}
	++semt->peticionesEnEsperaSelect;
	sem_post(&mutexPeticionesPorTabla);
	sem_wait(&semt->semaforoSelect);
}





void reiniciarSemaforos(char* tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexPeticionesPorTabla);
	SemaforoCompactacion *semt = (SemaforoCompactacion*) list_find(semaforosPorTabla, buscar);
	semt->peticionesEnEspera = 0;
	semt->peticionesEnEsperaSelect = 0;
	sem_post(&mutexPeticionesPorTabla);
}





void tomarTiempoInicio(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	((SemaforoCompactacion*) semt)->inicioBloqueo = getCurrentTime();
}





void tomarTiempoFin(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	void *semt = list_find(semaforosPorTabla, buscar);
	((SemaforoCompactacion*) semt)->finBloqueo = getCurrentTime();
}





void loggearTiempoCompactacion(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoCompactacion*) tablaSemaforo)->tabla);
	}
	SemaforoCompactacion *semt = (SemaforoCompactacion*)list_find(semaforosPorTabla, buscar);
	log_info(logger_visible, "Compactar(%s): Tiempo en el que la tabla estuvo bloqueada = %llu ms", tabla, semt->finBloqueo-semt->inicioBloqueo);
	log_info(logger_invisible, "Compactar(%s): Tiempo en el que la tabla estuvo bloqueada = %llu ms", tabla, semt->finBloqueo-semt->inicioBloqueo);
}





/*Request activas*/

void esperarSelectsActivos(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoRequestActivas*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexRequestActivas);
	SemaforoRequestActivas *semt = (SemaforoRequestActivas*)list_find(requestActivas, buscar);
	if(semt == NULL){
		sem_post(&mutexRequestActivas);
		return;
	}
	sem_post(&mutexRequestActivas);
	sem_wait(&semt->semaforoSelect);
}





void operacionTerminadaForSelects(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoRequestActivas*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexRequestActivas);
	SemaforoRequestActivas *semt = (SemaforoRequestActivas*)list_find(requestActivas, buscar);
	sem_post(&mutexRequestActivas);
	if(semt == NULL){
		return;
	}
	//semt->peticionesActivas = 0; deberia ser 0 sin necesidad de asignarlo
	sem_post(&semt->semaforoSelect);
}





void seVaAEjecutarRequestSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoRequestActivas*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexRequestActivas);
	SemaforoRequestActivas *semt = (SemaforoRequestActivas*)list_find(requestActivas, buscar);
	if(semt == NULL){
		sem_post(&mutexRequestActivas);
		return;
	}
	++semt->peticionesActivasSelect;
	int valorSemaforo;
	sem_getvalue(&semt->semaforoSelect, &valorSemaforo);
	sem_post(&mutexRequestActivas);
	if(valorSemaforo == 1)
		sem_wait(&semt->semaforoSelect);
}





void seTerminoDeEjecutarRequestSelect(char *tabla){
	bool buscar(void *tablaSemaforo){
		return !strcmp(tabla, ((SemaforoRequestActivas*) tablaSemaforo)->tabla);
	}
	sem_wait(&mutexRequestActivas);
	SemaforoRequestActivas *semt = (SemaforoRequestActivas*)list_find(requestActivas, buscar);
	if(semt == NULL){
		sem_post(&mutexRequestActivas);
		return;
	}
	--semt->peticionesActivasSelect;
	if(semt->peticionesActivasSelect == 0)
		sem_post(&semt->semaforoSelect);
	sem_post(&mutexRequestActivas);
}


