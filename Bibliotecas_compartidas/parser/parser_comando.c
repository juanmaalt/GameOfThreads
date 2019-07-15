#include "parser_comando.h"

#define RETURN_ERROR Comando ERROR={ .valido = false }; return ERROR

/*#define _CHECK_CLAVE x if(string_length(x) > 40){\
						fprintf(stderr, "La clave <%.40s...> es muy larga\n", (x)); \
						RETURN_ERROR; \
					   }*/ //En principio no interesa este chequeo


void destruir_comando(Comando op){
	if(op._raw){
		string_iterate_lines(op._raw, (void*)free);
		free(op._raw);
	}
}

Comando parsear_comando(char* line){
	if(line == NULL || string_equals_ignore_case(line, "")){
		fprintf(stderr, RED"No pude interpretar una linea vacia"STD"\n");
		RETURN_ERROR;
	}

	Comando ret = {
		.valido = true
	};

	char* auxLine = string_duplicate(line);
	string_trim(&auxLine);
	char** split = string_double_split(auxLine, " ", "\"");
	//char** split = string_n_split(auxLine, 5, " ");

	char* keyword = split[0];
	//char* clave = split[1];

	//Chequeos sintacticos
	if (keyword == NULL){
		fprintf(stderr, RED"Error sintactico, comando desconocido"STD"\n");
		RETURN_ERROR;
	}

	if(string_equals_ignore_case(keyword, "SELECT")){
		if(split[1] == NULL || split[2] == NULL){
			fprintf(stderr, RED"Error sintactico, SELECT [NOMBRE_TABLA(char*)] [KEY(int)]"STD"\n");
			RETURN_ERROR;
		}else{
			if(!esAlfaNumerica(split[1]) || !esNumerica(split[2])){
				fprintf(stderr, RED"Error sintactico, SELECT [NOMBRE_TABLA(char*)] [KEY(int)]"STD"\n");
				RETURN_ERROR;
			}
		}
	}

	if(string_equals_ignore_case(keyword, "INSERT")){
		if(split[1] == NULL || split[2] == NULL || split[3] == NULL){ //El insert puede no tener timestamp, es decir, split[4]
			fprintf(stderr, RED"Error sintactico, INSERT [NOMBRE_TABLA(cadena)] [KEY(numerico)] “[VALUE(cadena)]” [TIMESTAMP(numerico, opcional)]"STD"\n");
			RETURN_ERROR;
		}else{
			if(!esAlfaNumerica(split[1]) || !esNumerica(split[2]) || !esValue(split[3])){
				fprintf(stderr, RED"Error sintactico, INSERT [NOMBRE_TABLA(cadena)] [KEY(numerico)] “[VALUE(cadena)]” [TIMESTAMP(numerico, opcional)]"STD"\n");
				RETURN_ERROR;
			}
			else if(split[4] != NULL && !esNumerica(split[4])){
				fprintf(stderr, RED"Error sintactico, INSERT [NOMBRE_TABLA(char*)] [KEY(int)] “[VALUE(char*)]” [TIMESTAMP(int, opcional)]"STD"\n");
				RETURN_ERROR;
			}
		}
	}

	if(string_equals_ignore_case(keyword, "CREATE")){
		if(split[1] == NULL || split[2] == NULL || split[3] == NULL || split[4] == NULL){
			fprintf(stderr, RED"Error sintactico, CREATE [NOMBRE_TABLA(cadena)] [TIPO_CONSISTENCIA(SC|SHC|EC)] [NUMERO_PARTICIONES(numerico)] [COMPACTION_TIME(numerico)]"STD"\n");
			RETURN_ERROR;
		}else{
			if(!esAlfaNumerica(split[1]) || !esConsistenciaValida(split[2]) || !esNumerica(split[3]) || !esNumerica(split[4])){
				fprintf(stderr, RED"Error sintactico, CREATE [NOMBRE_TABLA(cadena)] [TIPO_CONSISTENCIA(SC|SHC|EC)] [NUMERO_PARTICIONES(numerico)] [COMPACTION_TIME(numerico)]"STD"\n");
				RETURN_ERROR;
			}
		}
	}

	if(string_equals_ignore_case(keyword, "DESCRIBE")){
		if(split[1] != NULL && !esAlfaNumerica(split[1])){
			fprintf(stderr, RED"Error sintactico, DESCRIBE [NOMBRE_TABLA(cadena, opcional)]"STD"\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "DROP")){
		if(split[1] == NULL){
			fprintf(stderr, RED"Error sintactico, DROP [NOMBRE_TABLA(cadena)]"STD"\n");
			RETURN_ERROR;
		}else{
			if(!esAlfaNumerica(split[1])){
				fprintf(stderr, RED"Error sintactico, DROP [NOMBRE_TABLA(cadena)]"STD"\n");
				RETURN_ERROR;
			}
		}
	}

	if(string_equals_ignore_case(keyword, "JOURNAL")){
		if(split[1] != NULL){
			fprintf(stderr, RED"Error sintactico, JOURNAL no lleva argumentos"STD"\n");
			RETURN_ERROR;
		}
	}

	if(string_equals_ignore_case(keyword, "ADD")){
		if(split[1] == NULL || split[2] == NULL || split[3] == NULL || split[4] == NULL){
			fprintf(stderr, RED"Error sintactico, ADD MEMORY [NÚMERO(int)] TO [CRITERIO(SC|SHC|EC)]"STD"\n");
			RETURN_ERROR;
		}else{
			if(!string_equals_ignore_case(split[1], "MEMORY")){
				fprintf(stderr, RED"Error sintactico, ADD MEMORY [NÚMERO(int)] TO [CRITERIO(SC|SHC|EC)]"STD"\n");
				RETURN_ERROR;
			}
			else if(!string_equals_ignore_case(split[3], "TO")){
				fprintf(stderr, RED"Error sintactico, ADD MEMORY [NÚMERO(int)] TO [CRITERIO(SC|SHC|EC)]"STD"\n");
				RETURN_ERROR;
			}
			else if(!esNumerica(split[2]) || !esConsistenciaValida(split[4])){
				fprintf(stderr, RED"Error sintactico, ADD MEMORY [NÚMERO(int)] TO [CRITERIO(SC|SHC|EC)]"STD"\n");
				RETURN_ERROR;
			}
		}
	}

	if(string_equals_ignore_case(keyword, "RUN")){
		if(split[1] == NULL){
			fprintf(stderr, RED"Error sintactico, RUN [PATH]"STD"\n");
			RETURN_ERROR;
		}else if(string_equals_ignore_case(split[1], "ALL")){
			if(split[2] == NULL){
				fprintf(stderr, RED"Error sintactico, RUN ALL [PATH]"STD"\n");
				RETURN_ERROR;
			}
		}
	}

	if(string_equals_ignore_case(keyword, "METRICS")){
		if(split[1] != NULL && !string_equals_ignore_case(split[1], "STOP")){
			fprintf(stderr, RED"Error sintactico, quizas quiso decir METRICS o METRICS STOP"STD"\n");
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
		remover_comillas(&split[3]);
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
		if(string_equals_ignore_case(split[1], "ALL")){
			ret.keyword = RUN_ALL;
			ret.argumentos.RUN_ALL.dirPath = split[2];
		}else{
			ret.keyword = RUN;
			ret.argumentos.RUN.path = split[1];
		}
	} else if(string_equals_ignore_case(keyword, "METRICS")){
		if(split[1] == NULL){
			ret.keyword = METRICS;
		}else if(string_equals_ignore_case(split[1], "STOP")){
			ret.keyword = METRICS_STOP;
		}
	} else {
		fprintf(stderr, RED"No se encontro el keyword <%s>"STD"\n", keyword); //Chequeo sintactico final
		RETURN_ERROR;
	}

	free(auxLine);
	return ret;
}

void comando_mostrar(Comando parsed){

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
                fprintf(stderr, RED"No se pude interpretar el enum: %d"STD"\n", parsed.keyword);
        }

        //destruir_operacion(parsed);
    } else {
        fprintf(stderr, RED"La linea no es valida"STD"\n");
    }
}

int comando_validar(Comando parsed){
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
    } else {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void remover_comillas(char** cadena){
	if(string_starts_with(*cadena, "\"")){
		char *temp = string_substring(*cadena, 1, strlen(*cadena));
		free(*cadena);
		*cadena = temp;
	}
	if(string_ends_with(*cadena, "\"")){
		char *temp = string_substring(*cadena, 0, strlen(*cadena)-1);
		free(*cadena);
		*cadena = temp;
	}
}

char *remover_new_line(char* cadena){
	char *retorno = string_from_format("%c", *cadena);
	for(int i=1; i<strlen(cadena); ++i){
		if(cadena[i]=='\n')
			continue;
		string_append(&retorno, string_from_format("%c", cadena[i]));
	}
	retorno = realloc(retorno, sizeof(char)*(strlen(retorno)+1));
	retorno[strlen(retorno)]='\0';
	return retorno;
}

bool esAlfaNumerica(char* cadena){
	for(int i=0; cadena[i]!='\n' && cadena[i]!='\0' && cadena[i]!=' '; ++i)
		if(!isalnum((int)cadena[i]))
			return false;
	return true;
}

bool esNumerica(char* cadena){
	for(int i=0; cadena[i]!='\n' && cadena[i]!='\0' && cadena[i]!=' '; ++i)
		if(!isdigit((int)cadena[i]))
			return false;
	return true;
}

bool esConsistenciaValida(char* cadena){
	if(string_equals_ignore_case(cadena, "SC") || string_equals_ignore_case(cadena, "EC") || string_equals_ignore_case(cadena, "SHC"))
		return true;
	return false;
}

bool esValue(char* cadena){
	for(int i=1; cadena[i]!='\n' && cadena[i]!='\0' && cadena[i]!='"'; ++i)
		if(cadena[i] != ' ')
			if(!isalnum((int)cadena[i]))
				return false;
	return true;
}

char **string_double_split(char *cadena, char *firstSeprator, char *secondSeparator){
	char **substrings = NULL;
	int size = 0;

	char *text_to_iterate = string_duplicate(cadena);

	char *next = text_to_iterate;
	char *str = text_to_iterate;

	while(next != NULL) {
		char* token = strtok_r(str, firstSeprator, &next);
		if(token == NULL) {
			break;
		}

		str = NULL;
		size++;
		substrings = realloc(substrings, sizeof(char*) * size);
		substrings[size - 1] = string_duplicate(token);

		if(string_starts_with(token, secondSeparator)){
			if(next != NULL){
				token = strtok_r(NULL, secondSeparator, &next);
				if(token == NULL)
					break;
				string_append(&substrings[size-1], string_from_format(" %s", token));
			}
		}
	};

	if (next[0] != '\0') {
		size++;
		substrings = realloc(substrings, sizeof(char*) * size);
		substrings[size - 1] = string_duplicate(next);
	}

	size++;
	substrings = realloc(substrings, sizeof(char*) * size);
	substrings[size - 1] = NULL;

	free(text_to_iterate);
	return substrings;
}





