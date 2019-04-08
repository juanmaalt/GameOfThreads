/*
 * Kernel.h
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>


t_log* iniciar_logger(void);
t_config* leer_config(void);
void leer_consola(t_log* logger);

void _leer_consola_haciendo(void(*accion)(char*));

#endif /* KERNEL_H_ */
