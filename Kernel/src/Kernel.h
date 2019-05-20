/*
 * Kernel.h
 *
 *  Created on: 8 abr. 2019
 *      Author: facundosalerno
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#define RED "\x1b[31m"
#define STD "\x1b[0m"

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
#include "Consola.h"
#include "Planificador.h"


//ESTRUCTURAS
struct Config_datos_fijos{
	const char *ip_memoria;
	const char *puerto_memoria;
	int multiprocesamiento;
};

typedef struct Config_datos_fijos fConfig;

struct Config_datos_variables{
	int (*quantum)();
	int (*refreshMetadata)();
	int (*retardo)();
}; //Se actualizan en tiempo de ejecucion

typedef struct Config_datos_variables vConfig;


//GLOBALES: Loggeo
t_log* logger_visible;
t_log* logger_invisible;
t_log* logger_error;

//GLOBALES: Config
t_config *configFile; //Contiene lo que sea que haya en el config
fConfig fconfig; //Contiene solo los datos fijos del config
vConfig vconfig; //Contiene solo los datos variables del config

//GLOBALES: Hilos y semaforos
pthread_t idConsola;
t_list *idsExecInstances; //TODO: ver como liberar esto al final del programa, poca importancia
sem_t disponibilidadPlanificador; //Para que la consola no pueda mandarle algo al planificador si no se inicio
sem_t scriptEnReady; //Para saber si hay algo en ready o no, y no estar preguntando permanentemente
sem_t extraerDeReadyDeAUno;
sem_t dormirProcesoPadre;
sem_t meterEnReadyDeAUno;


//FUNCIONES
int configuracion_inicial();
int iniciar_consola();
t_log* iniciar_logger(bool);
int inicializar_configs();
int extraer_quantum_config();
int extraer_refresMetadata_config();
int extraer_retardo_config();
void mostrar_por_pantalla_config(t_log* logger_visible);
void finalizar_todos_los_hilos();
void rutinas_de_finalizacion();

/*Descripcion de hilos:
 * padre (proceso): inicia todas las rutinas y termina haciendose cargo de la funcion ready() en planificador.c
 * consola: es iniciada por su padre y termina haciendose cargo de la consola en Consola.c. Va a ser el encargado de llamar a new() cada vez que reciba un script o comando
 * procesadores/unidades de ejecucion: tambien iniciados por el padre, todos se hacen cargo de la funcion exec()
 * */


#endif /* KERNEL_H_ */
