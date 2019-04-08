/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"


int main(void) {
	t_config* config = leer_config();


	return 0;

}

t_config* leer_config() {
	return config_create("Kernel.config");
}
