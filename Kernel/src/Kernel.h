/*
 * Kernel.h
 *
 *  Created on: 8 abr. 2019
 *      Author: facundosalerno
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#define STANDARD_PATH_KERNEL_CONFIG "Kernel.config"

#define RETURN_ERROR(mensaje) {if(logger_error != NULL && logger_invisible != NULL){ \
									log_error(logger_error, "%s", mensaje); \
									log_error(logger_invisible, "%s", mensaje); \
								}else{ \
									printf(RED"%s"STD"\n", mensaje); \
								} \
								return EXIT_FAILURE; \
							   }

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <cliente/cliente.h>
#include <parser/parser_comando.h>
#include <pthread.h>
#include <semaphore.h>
#include <colores/colores.h>
#include <sys/stat.h> //Para mkdir
#include <sys/inotify.h>
#include "Consola.h"
#include "Planificador.h"
#include "Metrics.h"


//ESTRUCTURAS
struct Config_datos_fijos{
	char *ip_memoria_principal;
	char *puerto_memoria_principal;
	int multiprocesamiento;
};

typedef struct Config_datos_fijos fConfig;

struct Config_datos_variables{
	int quantum;
	int refreshMetadata;
	int retardo;
}; //Se actualizan en tiempo de ejecucion

typedef struct Config_datos_variables vConfig;

//GLOBALES: Loggeo
t_log* logger_visible;
t_log* logger_invisible;
t_log* logger_error;

//GLOBALES: Config
fConfig fconfig; //Contiene solo los datos fijos del config
vConfig vconfig; //Contiene solo los datos variables del config

//GLOBALES: Hilos y semaforos
pthread_t idConsola;
pthread_t servicioMetricas;
pthread_t gossiping;
pthread_t describeAutomatico;
pthread_t inotify;
t_list *idsExecInstances;
sem_t disponibilidadPlanificador; //Para que la consola no pueda mandarle algo al planificador si no se inicio
sem_t scriptEnReady; //Para saber si hay algo en ready o no, y no estar preguntando permanentemente
sem_t dormirProcesoPadre;

//GLOBALES: semaforos mutex: solo se sincroniza la escritura, no la lectura
sem_t mutexColaReady;
sem_t mutexMemoriasSC;
sem_t mutexMemoriasHSC;
sem_t mutexMemoriasEC;
sem_t mutexMemoriasExistentes;
sem_t mutexTablasExistentes;
sem_t mutexMetricas;


//FUNCIONES: Publicas
void mostrar_por_pantalla_config();


#endif /* KERNEL_H_ */
