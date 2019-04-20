/*
 * Memoria.h
 *
 *  Created on: 13 abr. 2019
 *      Author:	fdalmaup
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <cliente/cliente.h>

//ESTRUCTURAS
typedef struct{
	char *puerto;
	char *ip_fileSystem;
	char *puerto_fileSystem;
	char *ip_seeds;
	char *puerto_seeds;
	char *tamanio_memoria;
	char *numero_memoria;
}Config_final_data;

//FUNCIONES
t_log* iniciar_logger();
t_config* leer_config(void);
void get_data_config(Config_final_data *, t_config* );
void ver_config(Config_final_data *config, t_log* logger_visible);


#endif /* MEMORIA_H_ */
