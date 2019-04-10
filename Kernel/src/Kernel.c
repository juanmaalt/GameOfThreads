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
	Comando parsed = parse("ADD MEMORY num1 TO crit1");
	//Comando parsed = parse("RUN unlugar");

    if(parsed.valido){
        switch(parsed.keyword){
            case SELECT:
                printf("SELECT\n");
                printf("nombreTabla: %s\n", parsed.argumentos.SELECT.nombreTabla);
                printf("key: %s\n", parsed.argumentos.SELECT.key);
                break;
            case INSERT:
                printf("INSERT\n");
                printf("nombreTabla: %s\n", parsed.argumentos.INSERT.nombreTabla);
                printf("key: %s\n", parsed.argumentos.INSERT.key);
                printf("value: %s\n", parsed.argumentos.INSERT.value);
                printf("timestamp (opcional): %s\n", parsed.argumentos.INSERT.timestamp);
                break;
            case CREATE:
                printf("CREATE\n");
                printf("nombreTabla: %s\n", parsed.argumentos.CREATE.nombreTabla);
                printf("tipoConsistencia: %s\n", parsed.argumentos.CREATE.tipoConsistencia);
                printf("numeroParticiones: %s\n", parsed.argumentos.CREATE.numeroParticiones);
                printf("compactacionTime: %s\n", parsed.argumentos.CREATE.compactacionTime);
                break;
            case DESCRIBE:
                printf("DESCRIBE\n");
                printf("nombreTabla: %s\n", parsed.argumentos.DESCRIBE.nombreTabla);
                break;
            case DROP:
            	printf("DROP\n");
                printf("nombreTabla: %s\n", parsed.argumentos.DROP.nombreTabla);
                break;
            case JOURNAL:
                printf("JOURNAL\nno posee argumentos\n");
                break;
            case ADDMEMORY:
            	printf("ADD MEMORY\n");
            	printf("numero: %s\n", parsed.argumentos.ADDMEMORY.numero);
            	printf("criterio: %s\n", parsed.argumentos.ADDMEMORY.criterio);
                break;
            case RUN:
            	printf("RUN\n");
            	printf("path: %s\n", parsed.argumentos.RUN.path);
                break;
            default:
                fprintf(stderr, "No se pude interpretar\n");
                exit(EXIT_FAILURE);
        }

        destruir_operacion(parsed);
    } else {
        fprintf(stderr, "La linea no es valida\n");
        exit(EXIT_FAILURE);
    }

	return 0;

}

t_config* leer_config() {
	return config_create("Kernel.config");
}
