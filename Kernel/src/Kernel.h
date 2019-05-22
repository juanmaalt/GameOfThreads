/*
 * Kernel.h
 *
 *  Created on: 8 abr. 2019
 *      Author: facundosalerno
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#define STANDARD_PATH_KERNEL_CONFIG "Kernel.config"

#define RETURN_ERROR(mensaje) {logger_error != NULL ? log_error(logger_error, "%s", mensaje) : printf(RED"%s"STD"\n", mensaje); return EXIT_FAILURE;}

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
#include "Consola.h"
#include "Planificador.h"


//ESTRUCTURAS
struct Config_datos_fijos{
	char *ip_memoria;
	char *puerto_memoria;
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


//FUNCIONES: Publicas
void mostrar_por_pantalla_config();

//FUNCIONES: Privadas
static int configuracion_inicial();
static int iniciar_consola();
static t_log* iniciar_logger(char *fileName, bool visibilidad, t_log_level level);
static int inicializar_configs();
static int extraer_quantum_config();
static int extraer_refresMetadata_config();
static int extraer_retardo_config();
static void finalizar_todos_los_hilos();
static void rutinas_de_finalizacion();

/*Descripcion de hilos:
 * padre (proceso): inicia todas las rutinas y termina haciendose cargo de la funcion ready() en planificador.c
 * consola: es iniciada por su padre y termina haciendose cargo de la consola en Consola.c. Va a ser el encargado de llamar a new() cada vez que reciba un script o comando
 * procesadores/unidades de ejecucion: tambien iniciados por el padre, todos se hacen cargo de la funcion exec()
 * */


#endif /* KERNEL_H_ */
