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

	//Comando parsed = parse("SELECT tabla1 key1");
	//Comando parsed = parse("INSERT tabla1 key1 value1 timestamp1");
	//Comando parsed = parse("CREATE tabla1 consis partic compac");
	//Comando parsed = parse("DESCRIBE tabla1");
	//Comando parsed = parse("DROP tabla1");
	//Comando parsed = parse("JOURNAL"); //REVISAR
	//Comando parsed = parse("ADD MEMORY num1 TO crit1");
	//Comando parsed = parse("RUN unlugar");

    //mostrar(parsed);

	int miSocket = enable_server("8002", "127.0.0.1");
	waiting_conections(miSocket);

	return 0;

}

t_config* leer_config() {
	return config_create("Kernel.config");
}
