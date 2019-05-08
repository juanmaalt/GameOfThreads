/*
 * Kernel.h
 *
 *  Created on: 8 abr. 2019
 *      Author: facundosalerno
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <cliente/cliente.h>
#include <parser/parser_comando.h>
#include <signal.h> //Para SIGTERM
#include <sys/wait.h> //Para wait(), esperar a que un proceso hijo termine para finalizar
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
int pidConsola;
t_list *pidsProcesadores;


//FUNCIONES
int configuracion_inicial();
int iniciar_consola();
int iniciar_planificacion();
void finalizar_procesos_hijos();
void esperar_procesos_hijos();
t_log* iniciar_logger(bool);
t_config* leer_config();
void extraer_data_config(Config_final_data *config, t_config* configFile);
void ver_config(Config_final_data *config, t_log* logger_visible);


#endif /* KERNEL_H_ */
