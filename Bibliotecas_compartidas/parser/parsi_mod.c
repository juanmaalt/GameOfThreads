#include "parsi_mod.h"

#define RETURN_ERROR Comando ERROR={ .valido = false }; return ERROR

/*#define _CHECK_CLAVE x if(string_length(x) > 40){\
						fprintf(stderr, "La clave <%.40s...> es muy larga\n", (x)); \
						RETURN_ERROR; \
					   }*/ //En principio no interesa este chequeo


void destruir_operacion(Comando op){
	if(op._raw){
		string_iterate_lines(op._raw, (void*) free);
		free(op._raw);
	}
}

Comando parse(char* line){
	if(line == NULL || string_equals_ignore_case(line, "")){
		fprintf(stderr, "No pude interpretar una linea vacia\n");
		RETURN_ERROR;
	}

	Comando ret = {
		.valido = true
	};

	char* auxLine = string_duplicate(line);
	string_trim(&auxLine);
	char** split = string_n_split(auxLine, 5, " ");

	char* keyword = split[0];
	//char* clave = split[1];

	//Chequeos sintacticos
	if (keyword == NULL){
		fprintf(stderr, "Error sintactico, comando desconocido\n");
		RETURN_ERROR;
	}

	if(string_equals_ignore_case(keyword, "SELECT")){
		if(split[1] == NULL || split[2] == NULL){
			fprintf(stderr, "Error sintactico, argumentos para SELECT invalidos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "INSERT")){
		if(split[1] == NULL || split[2] == NULL || split[3] == NULL){ //El insert puede no tener timestamp, es decir, split[4]
			fprintf(stderr, "Error sintactico, argumentos para INSERT invalidos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "CREATE")){
		if(split[1] == NULL || split[2] == NULL || split[3] == NULL || split[4] == NULL){
			fprintf(stderr, "Error sintactico, argumentos para CREATE invalidos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "DESCRIBE")){
		if(split[1] == NULL){
			fprintf(stderr, "Error sintactico, argumentos para DESCRIBE invalidos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "DROP")){
		if(split[1] == NULL){
			fprintf(stderr, "Error sintactico, argumentos para DROP invalidos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "JOURNAL")){
		if(split[1] != NULL){
			fprintf(stderr, "Error sintactico, JOURNAL no lleva argumentos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "ADD")){
		if(split[1] == NULL || split[2] == NULL || split[3] == NULL || split[4] == NULL){
			fprintf(stderr, "Error sintactico, argumentos para ADD MEMORY invalidos\n");
			RETURN_ERROR;
		}
		if(!string_equals_ignore_case(split[1], "MEMORY")){
			fprintf(stderr, "Error sintactico, tal vez olvido el 'MEMORY'\n");
			RETURN_ERROR;
		}
		if(!string_equals_ignore_case(split[3], "TO")){
			fprintf(stderr, "Error sintactico, tal vez olvido el 'TO'\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "RUN")){
		if(split[1] == NULL){
			fprintf(stderr, "Error sintactico, argumentos para RUN invalidos\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "METRICS")){
		if(split[1] != NULL){
			fprintf(stderr, "Error sintactico, METRICS no lleva argumentos\n");
			RETURN_ERROR;
		}
	}

	/*if(string_length(clave) > 40){
		fprintf(stderr, "La clave <%.40s...> es muy larga\n");
		RETURN_ERROR; 
	}*/

	//Fin chequeos sintacticos
	
	ret._raw = split;

	if(string_equals_ignore_case(keyword, "SELECT")){
		ret.keyword = SELECT;
		ret.argumentos.SELECT.nombreTabla = split[1];
		ret.argumentos.SELECT.key = split[2];
	} else if(string_equals_ignore_case(keyword, "INSERT")){
		ret.keyword = INSERT;
		ret.argumentos.INSERT.nombreTabla =  split[1];
		ret.argumentos.INSERT.key =  split[2];
		ret.argumentos.INSERT.value =  split[3];
		ret.argumentos.INSERT.timestamp =  split[4];
	} else if(string_equals_ignore_case(keyword, "CREATE")){
		ret.keyword = CREATE;
		ret.argumentos.CREATE.nombreTabla = split[1];
		ret.argumentos.CREATE.tipoConsistencia = split[2];
		ret.argumentos.CREATE.numeroParticiones = split[3];
		ret.argumentos.CREATE.compactacionTime = split[4];
	} else if(string_equals_ignore_case(keyword, "DESCRIBE")){
		ret.keyword = DESCRIBE;
		ret.argumentos.DESCRIBE.nombreTabla = split[1];
	} else if(string_equals_ignore_case(keyword, "DROP")){
		ret.keyword = DROP;
		ret.argumentos.DROP.nombreTabla = split[1];
	} else if(string_equals_ignore_case(keyword, "JOURNAL")){
		ret.keyword = JOURNAL;
	} else if(string_equals_ignore_case(keyword, "ADD")){
		ret.keyword = ADDMEMORY;
		ret.argumentos.ADDMEMORY.numero = split[2];
		ret.argumentos.ADDMEMORY.criterio = split[4];
	} else if(string_equals_ignore_case(keyword, "RUN")){
		ret.keyword = RUN;
		ret.argumentos.RUN.path = split[1];
	} else if(string_equals_ignore_case(keyword, "METRICS")){
		ret.keyword = METRICS;
	} else {
		fprintf(stderr, "No se encontro el keyword <%s>\n", keyword); //Chequeo sintactico final
		RETURN_ERROR;
	}

	free(auxLine);
	return ret;
}

void parsi_mostrar(Comando parsed){

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
            case METRICS:
            	printf("METRICS\nno posee argumentos\n");
                break;
            default:
                fprintf(stderr, "No se pude interpretar el enum: %d\n", parsed.keyword);
        }

        //destruir_operacion(parsed);
    } else {
        fprintf(stderr, "La linea no es valida\n");
    }
}

int parsi_validar(Comando parsed){
    if(parsed.valido){
        switch(parsed.keyword){
            case SELECT:
            case INSERT:
            case CREATE:
            case DESCRIBE:
            case DROP:
            case JOURNAL:
            case ADDMEMORY:
            case RUN:
            case METRICS:
                break;
            default:
                return EXIT_FAILURE;
        }
        //destruir_operacion(parsed);
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
