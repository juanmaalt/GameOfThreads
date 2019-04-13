/*
 * LFS.h
 *
 *  Created on: 9 abr. 2019
 *      Author: juanmaalt
 */

#ifndef LFS_H_
#define LFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <cliente/cliente.h>

//FUNCIONES
void levantar_config(void);
void ver_config(void);

//ESTRUCTURAS
typedef struct{
	char *puerto_escucha;
	char *punto_montaje;
	char *retardo;
	char *tamanio_value;
	char *tiempo_dump;
}Config_data;

//GLOBALES
Config_data config;


#endif /* LFS_H_ */
