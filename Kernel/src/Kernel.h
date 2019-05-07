/*
 * Kernel.h
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <cliente/cliente.h>
#include <parser/parser_comando.h>

//ESTRUCTURAS
typedef struct{
	char *ip_memoria;
	char *puerto_memoria;
	char *multiprocesamiento;
}Config_final_data;


//FUNCIONES
t_log* iniciar_logger(bool);
t_config* leer_config();
void get_data_config(Config_final_data *config, t_config* configFile);
void ver_config(Config_final_data *config, t_log* logger_visible);
extern int serve_client(int socketCliente);
void leer_consola(t_log* logger);
void _leer_consola_haciendo(void(*accion)(char*));
int funcion_conexion(int socket);

#endif /* KERNEL_H_ */
