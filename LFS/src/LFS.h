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
#include <commons/log.h>
#include <cliente/cliente.h>

//ESTRUCTURAS
typedef struct{
	char *ip;
	char *puerto_escucha;
	char *ip_memoria;
	char *puerto_memoria;
	char *punto_montaje;
	//char *retardo; //Esta config la dejamos como variable.
	char *tamanio_value;
	//char *tiempo_dump; //Esta config la dejamos como variable.
}Config_final_data;


//FUNCIONES
t_log* iniciar_logger(bool);
t_config* leer_config();
void extraer_data_config(Config_final_data *config, t_config* configFile);
void ver_config(Config_final_data *config, t_log* logger_visible);
void handshakeMemoria(int socket);
int threadConnection(int serverSocket, void *funcionThread);

//API
void selectAPI(Comando);
void insertAPI(Comando);
void createAPI(Comando);
void describeAPI(Comando);
void dropAPI(Comando);


#endif /* LFS_H_ */
