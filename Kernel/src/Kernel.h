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
typedef struct{
	char *ip_memoria;
	char *puerto_memoria;
	int quantum;
	int multiprocesamiento;
	int refreshMetadata;
	int retardo;
}Config_final_data;


//GLOBALES
t_log* logger_visible;
t_log* logger_invisible;
Config_final_data config;
pthread_t idConsola;
t_list *pidsProcesadores; //TODO: ver como liberar esto al final del programa, poca importancia
sem_t disponibilidadPlanificador; //Para que la consola no pueda mandarle algo al planificador si no se inicio
sem_t scriptEnReady; //Para saber si hay algo en ready o no, y no estar preguntando permanentemente


//FUNCIONES
int configuracion_inicial();
int iniciar_consola();
void finalizar_procesos_hijos();
void esperar_procesos_hijos();
t_log* iniciar_logger(bool);
t_config* leer_config();
void extraer_data_config(Config_final_data *config, t_config* configFile);
void ver_config(Config_final_data *config, t_log* logger_visible);

/*Descripcion de hilos:
 * padre (proceso): inicia todas las rutinas y termina haciendose cargo de la funcion ready() en planificador.c
 * consola: es iniciada por su padre y termina haciendose cargo de la consola en Consola.c. Va a ser el encargado de llamar a new() cada vez que reciba un script o comando
 * procesadores/unidades de ejecucion: tambien iniciados por el padre, todos se hacen cargo de la funcion exec()
 * */


#endif /* KERNEL_H_ */
