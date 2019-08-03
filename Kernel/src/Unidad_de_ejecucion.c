/*
 * Unidad_de_ejecucion.c
 *
 *  Created on: 8 may. 2019
 *      Author: facundosalerno
 */

#include "Unidad_de_ejecucion.h"

//FUNCIONES: Privadas. No van en el header.
static INTERNAL_STATE exec_file_lql(PCB *pcb);
static INTERNAL_STATE exec_string_comando(PCB *pcb);
static INTERNAL_STATE procesar_retorno_operacion(Operacion op, PCB *pcb, char *instruccionActual, DynamicAddressingRequest* link);//Recibe el retorno de operacion (lo mas importante). El pcb es para mostrar datos extras como el nombre del archivo que fallo, al igual que la instruccion actual
static DynamicAddressingRequest direccionar_request(char *request);
static socket_t comunicarse_con_memoria();
static socket_t comunicarse_con_memoria_principal();
static void generar_estadisticas(DynamicAddressingRequest *link);


void *exec(void *null){
	pthread_detach(pthread_self());
	for(;;){
		//La cpu por default esta disponible si esta en este wait
		sem_wait(&scriptEnReady);
		sem_wait(&mutexColaReady);
		PCB *pcb = seleccionar_siguiente();
		sem_post(&mutexColaReady);
		switch(pcb->tipo){
		case STRING_COMANDO:
			exec_string_comando(pcb);
			break;
		case FILE_LQL:
			exec_file_lql(pcb);
			break;
		}
	}
	return NULL;
}





static DynamicAddressingRequest direccionar_request(char *request){
	DynamicAddressingRequest retorno;
	retorno.socket = 0;
	if(USAR_SOLO_MEMORIA_PRINCIPAL){
		retorno.socket = comunicarse_con_memoria_principal();
		return retorno;
	}
	Comando comando = parsear_comando(request);
	Memoria *memoria=NULL;
	switch(comando.keyword){//A esta altura ya nos aseguramos de que el comando habia sido valido
	case SELECT:
		memoria = determinar_memoria_para_tabla(comando.argumentos.SELECT.nombreTabla, comando.argumentos.SELECT.key);
		retorno.criterioQueSeUso = consistencia_de_tabla(comando.argumentos.SELECT.nombreTabla);
		retorno.tipoOperacion = LECTURA;
		break;
	case INSERT:
		memoria = determinar_memoria_para_tabla(comando.argumentos.INSERT.nombreTabla, comando.argumentos.INSERT.key);
		retorno.criterioQueSeUso = consistencia_de_tabla(comando.argumentos.INSERT.nombreTabla);
		retorno.tipoOperacion = ESCRITURA;
		break;
	case CREATE:
		if(!tabla_esta_en_la_lista(comando.argumentos.CREATE.nombreTabla))
			agregar_metadata_tabla(comando.argumentos.CREATE.nombreTabla, comando.argumentos.CREATE.tipoConsistencia, comando.argumentos.CREATE.numeroParticiones, comando.argumentos.CREATE.compactacionTime);
		memoria = determinar_memoria_para_tabla(comando.argumentos.CREATE.nombreTabla, NULL);
		retorno.criterioQueSeUso = consistencia_de_tabla(comando.argumentos.CREATE.nombreTabla);
		retorno.tipoOperacion = ESCRITURA;
		break;
	case DESCRIBE:
		if(comando.argumentos.DESCRIBE.nombreTabla == NULL){
			memoria = elegir_cualquiera();
			retorno.criterioQueSeUso = NULL_CONSISTENCY; //Para que no joda en las estadisticas
		}else{
			memoria = determinar_memoria_para_tabla(comando.argumentos.DESCRIBE.nombreTabla, NULL);
			retorno.criterioQueSeUso = consistencia_de_tabla(comando.argumentos.DESCRIBE.nombreTabla);
		}
		retorno.tipoOperacion = LECTURA;
		break;
	case DROP:
		memoria = determinar_memoria_para_tabla(comando.argumentos.DROP.nombreTabla, NULL);
		retorno.criterioQueSeUso = consistencia_de_tabla(comando.argumentos.DROP.nombreTabla);
		retorno.tipoOperacion = ESCRITURA; //TODO: podria sacar la tabla de mi lista
		break;
	default:
		log_info(logger_invisible, "Instruccion ilegal");
		log_info(logger_visible, "Instruccion ilegal");
		memoria = NULL;
	}
	destruir_comando(comando);

	if(memoria == NULL){
		if(DELEGAR_A_MEMORIA_PRINCIPAL){
			printf(YEL"Warning: la request '%s' se delego a la memoria principal\n"STD, request);
			log_info(logger_invisible, "Warning: la request '%s' se delego a la memoria principal", request);
			retorno.socket = comunicarse_con_memoria_principal();
			return retorno;
		}
		retorno.socket = NULL_MEMORY;
		return retorno;
	}
	retorno.memoria = memoria;
	retorno.socket = comunicarse_con_memoria(memoria);
	if(retorno.socket == EXIT_FAILURE){
		log_error(logger_error, "Unidad_de_ejecucion.c: direccionar_request: la memoria elegida parece estar caida. Se eligira otra");
		log_error(logger_invisible, "Unidad_de_ejecucion.c: direccionar_request: la memoria elegida parece estar caida. Se eligira otra");
		remover_memoria(memoria);
		if(list_is_empty(memoriasExistentes))
			retorno.socket = NULL_MEMORY;
	}
	return retorno;
}





static socket_t comunicarse_con_memoria(Memoria *memoria){
	int socketServer;
	if((socketServer = connect_to_server(memoria->ip, memoria->puerto)) == EXIT_FAILURE){
		log_error(logger_error, "Unidad_de_ejecucion.c: comunicarse_con_memoria: error al conectarse al servidor memoria %s:%s", memoria->ip, memoria->puerto);
		log_error(logger_invisible, "Unidad_de_ejecucion.c: comunicarse_con_memoria: error al conectarse al servidor memoria %s:%s", memoria->ip, memoria->puerto);
		return EXIT_FAILURE;
	}
	log_info(logger_invisible, "Request dirigida a la memoria numero: %d, %s:%s", memoria->numero, memoria->ip, memoria->puerto);
	//log_info(logger_visible, "Request dirigida a la memoria numero: %d, %s:%s", memoria->numero, memoria->ip, memoria->puerto);
	return socketServer;
}


int solicitar_socket(Memoria *memoria){ //Si la cpu lo permite
	return comunicarse_con_memoria(memoria);
}


static socket_t comunicarse_con_memoria_principal(){
	int socketServer;
	if((socketServer = connect_to_server(fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal)) == EXIT_FAILURE){
		log_error(logger_error, "Unidad_de_ejecucion.c: comunicarse_con_memoria_principal: error al conectarse al servidor memoria %s:%s", fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
		log_error(logger_invisible, "Unidad_de_ejecucion.c: comunicarse_con_memoria_principal: error al conectarse al servidor memoria %s:%s", fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
		return EXIT_FAILURE;
	}
	log_info(logger_invisible, "Request dirigida a la memoria principal %s:%s", fconfig.ip_memoria_principal, fconfig.puerto_memoria_principal);
	return socketServer;
}





static INTERNAL_STATE exec_string_comando(PCB *pcb){
	DynamicAddressingRequest target;
	do{
		target = direccionar_request((char *)pcb->data);
	}while(target.socket == EXIT_FAILURE);
	if(target.socket == NULL_MEMORY){
		free(pcb->data);
		free(pcb->nombreArchivoLQL);
		free(pcb);
		log_error(logger_error, "Unidad_de_ejecucion.c: exec_string_comando: Finalizando operacion. No hay memorias para ejecutar la instruccion");
		log_error(logger_invisible, "Unidad_de_ejecucion.c: exec_string_comando: Finalizando operacion. No hay memorias para ejecutar la instruccion");
		return INSTRUCCION_ERROR;
	}
	target.inicioOperacion = getCurrentTime();
	Operacion request;
	request.TipoDeMensaje = COMANDO;
	request.Argumentos.COMANDO.comandoParseable = string_from_format((char*)pcb->data);
	send_msg(target.socket, request);
	destruir_operacion(request);
	request = recv_msg(target.socket);
	int retorno = procesar_retorno_operacion(request, pcb, (char*)pcb->data, &target);
	if(retorno != INSTRUCCION_ERROR && retorno != JOURNAL_MEMORY_INACCESSIBLE){
		target.operacionExitosa = true;
		target.finOperacion = getCurrentTime();
		sem_wait(&mutexMetricas);
		generar_estadisticas(&target);
		sem_post(&mutexMetricas);
	}

	destruir_operacion(request);
	free(pcb->data);
	free(pcb->nombreArchivoLQL);
	free(pcb);
	close(target.socket);
	simular_retardo();
	return FINISH;
}





static INTERNAL_STATE exec_file_lql(PCB *pcb){
	Operacion request;
	char buffer[MAX_BUFFER_SIZE_FOR_LQL_LINE];
	char *line = NULL;
	FILE *lql = (FILE *)pcb->data; //Como el FILE nunca se cerro, cada vez que entre, va a continuar donde se habia quedado
	sem_wait(&mutexQuantumRefresh);
	int quantumBuffer = vconfig.quantum; //Para hacer la llamada una sola vez por cada exec. No se actualiza el quantum en tiempo real, pero se actualiza cuando entra un nuevo script por que ya tiene el valor actualizado
	sem_post(&mutexQuantumRefresh);

	for(int i=1; i<=quantumBuffer; ++i){
		fgetpos(lql, &(pcb->instruccionPointer));
		line = fgets(buffer, MAX_BUFFER_SIZE_FOR_LQL_LINE, lql);
		if(line == NULL || feof(lql)){
			log_info(logger_visible, GRN"Unidad_de_ejecucion.c: exec_file_lql: El script %s finalizo"STD, pcb->nombreArchivoLQL);
			log_info(logger_invisible, GRN"Unidad_de_ejecucion.c: exec_file_lql: El script %s finalizo"STD, pcb->nombreArchivoLQL);
			printf("\n");
			fclose(lql);
			free(pcb->nombreArchivoLQL);
			free(pcb);
			return FINISH;
		}
		DynamicAddressingRequest target;
		do{
			target = direccionar_request(line);
		}while(target.socket == EXIT_FAILURE);
		if(target.socket == NULL_MEMORY){
			printf("\n");
			char *aux = remover_new_line(line);
			log_error(logger_error, "Unidad_de_ejecucion.c: exec_file_lql: finalizando operacion '%s' debido a la instruccion '%s'. No hay memorias para ejecutar la instruccion", pcb->nombreArchivoLQL, aux);
			log_error(logger_invisible, "Unidad_de_ejecucion.c: exec_file_lql: finalizando operacion '%s' debido a la instruccion '%s'. No hay memorias para ejecutar la instruccion", pcb->nombreArchivoLQL, aux);
			free(aux);
			fclose(lql);
			free(pcb->nombreArchivoLQL);
			free(pcb);
			return INSTRUCCION_ERROR;
		}
		target.inicioOperacion = getCurrentTime();
		request.TipoDeMensaje = COMANDO;
		request.Argumentos.COMANDO.comandoParseable = string_from_format(line);
		send_msg(target.socket, request);
		destruir_operacion(request);
		request = recv_msg(target.socket);
		int retorno = procesar_retorno_operacion(request, pcb, line, &target);
		if(retorno == INSTRUCCION_ERROR){
			destruir_operacion(request);
			fclose(lql);
			free(pcb->nombreArchivoLQL);
			free(pcb);
			close(target.socket);
			return INSTRUCCION_ERROR;
		}
		if(retorno != JOURNAL_MEMORY_INACCESSIBLE){
			target.operacionExitosa = true;
			target.finOperacion = getCurrentTime();
			sem_wait(&mutexMetricas);
			generar_estadisticas(&target);
			sem_post(&mutexMetricas);
		}
		destruir_operacion(request);
		close(target.socket);
	}
	printf("\n");
	sem_wait(&mutexColaReady);
	desalojar(pcb);
	sem_post(&mutexColaReady);
	sem_post(&scriptEnReady); //Ya que se metio a la lista de vuelta
	simular_retardo();
	return DESALOJO;
}





static INTERNAL_STATE procesar_retorno_operacion(Operacion op, PCB* pcb, char* instruccionActual, DynamicAddressingRequest* link){
	char *instruccionActualTemp = NULL;
	switch(op.TipoDeMensaje){
	case TEXTO_PLANO:
		instruccionActualTemp = remover_new_line(instruccionActual);
		log_info(logger_visible,"CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, ++pcb->simbolicIP, instruccionActualTemp, op.Argumentos.TEXTO_PLANO.texto);
		log_info(logger_invisible,"CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp, op.Argumentos.TEXTO_PLANO.texto);
		free(instruccionActualTemp);
		return CONTINUE;
	case REGISTRO:
		instruccionActualTemp = remover_new_line(instruccionActual);
		log_info(logger_visible,"CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> [Timestamp: %llu, Key: %d, Value: %s]", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, ++pcb->simbolicIP, instruccionActualTemp, op.Argumentos.REGISTRO.timestamp, op.Argumentos.REGISTRO.key, op.Argumentos.REGISTRO.value);
		log_info(logger_invisible,"CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> [Timestamp: %llu, Key: %d, Value: %s]", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp, op.Argumentos.REGISTRO.timestamp, op.Argumentos.REGISTRO.key, op.Argumentos.REGISTRO.value);
		free(instruccionActualTemp);
		return CONTINUE;
	case ERROR:
		instruccionActualTemp = remover_new_line(instruccionActual);
		log_error(logger_error,"CPU: %d | Memoria: %d %s:%s | %s (%d) | Fallo en la instruccion '%s'. Abortando: %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, ++pcb->simbolicIP,  instruccionActualTemp, op.Argumentos.ERROR.mensajeError);
		log_error(logger_invisible,"CPU: %d | Memoria: %d %s:%s | %s (%d) | Fallo en la instruccion '%s'. Abortando: %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp, op.Argumentos.ERROR.mensajeError);
		free(instruccionActualTemp);
		return INSTRUCCION_ERROR;
	case ERROR_JOURNAL: //Ya no se usa este case
		instruccionActualTemp = remover_new_line(instruccionActual);
		if(pcb->tipo == FILE_LQL){
			fsetpos((FILE *)pcb->data, &(pcb->instruccionPointer));
			log_info(logger_visible,YEL"CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> La memoria esta realizando Journal. Se eligira otra de manera aleatoria si el criterio lo permite"STD, process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp);
			log_info(logger_invisible,"CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> La memoria esta realizando Journal. Se eligira otra de manera aleatoria si el criterio lo permite", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp);
			free(instruccionActualTemp);
			//sleep(3);
			return JOURNAL_MEMORY_INACCESSIBLE;
		}
		log_error(logger_error, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> La memoria esta realizando Journal. Se eligira otra de manera aleatoria si el criterio lo permite", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, ++pcb->simbolicIP, instruccionActualTemp);
		log_error(logger_invisible, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> La memoria esta realizando Journal. Se eligira otra de manera aleatoria si el criterio lo permite", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp);
		free(instruccionActualTemp);
		return INSTRUCCION_ERROR;
	case ERROR_MEMORIAFULL:
		instruccionActualTemp = remover_new_line(instruccionActual);
		log_error(logger_error, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> La memoria esta full. Realizar journal.", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP+1, instruccionActualTemp);
		log_error(logger_invisible, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> La memoria esta full. Realizar journal.", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP+1, instruccionActualTemp);
		free(instruccionActualTemp);
		destruir_operacion(op);
		op = recv_msg(link->socket);
		return procesar_retorno_operacion(op, pcb, instruccionActual, link);
	case DESCRIBE_REQUEST:
		if(op.Argumentos.DESCRIBE_REQUEST.esGlobal){
			if(procesar_describe_global(op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido) == EXIT_FAILURE){
				log_error(logger_error,"CPU: %d | %s (%d) | Abortando: Fallo la descompresion del Describe", process_get_thread_id(), pcb->nombreArchivoLQL, ++pcb->simbolicIP);
				log_error(logger_invisible,"CPU: %d | %s (%d) | Abortando: Fallo la descompresion del Describe", process_get_thread_id(), pcb->nombreArchivoLQL, pcb->simbolicIP);
				return INSTRUCCION_ERROR;
			}
		}else{
			if(procesar_describe_simple(op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido, instruccionActual) == EXIT_FAILURE){
				log_error(logger_error,"CPU: %d | %s (%d) | Abortando: Fallo la descompresion del Describe", process_get_thread_id(), pcb->nombreArchivoLQL, ++pcb->simbolicIP);
				log_error(logger_invisible,"CPU: %d | %s (%d) | Abortando: Fallo la descompresion del Describe", process_get_thread_id(), pcb->nombreArchivoLQL, pcb->simbolicIP);
				return INSTRUCCION_ERROR;
			}
		}
		instruccionActualTemp = remover_new_line(instruccionActual);
		log_info(logger_invisible, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, ++pcb->simbolicIP, instruccionActualTemp, op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
		if(pcb->tipo == FILE_LQL){
			log_info(logger_visible, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp, op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
		}else{
			log_info(logger_visible, "CPU: %d | Memoria: %d %s:%s | %s (%d) | %s -> %s", process_get_thread_id(), link->memoria->numero, link->memoria->ip, link->memoria->puerto, pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp, op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);
			mostrar_describe(op.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);

		}
		free(instruccionActualTemp);
		return CONTINUE;
	default:
		instruccionActualTemp = remover_new_line(instruccionActual);
		log_error(logger_error,"CPU: %d | %s (%d) | Instruccion ilegal '%s' invalida o fuera de contexto, Path: %s", process_get_thread_id(), pcb->nombreArchivoLQL, ++pcb->simbolicIP, instruccionActualTemp, pcb->nombreArchivoLQL);
		log_error(logger_invisible,"CPU: | %s (%d) | Instruccion ilegal '%s' invalida o fuera de contexto, Path: %s", process_get_thread_id(), pcb->nombreArchivoLQL, pcb->simbolicIP, instruccionActualTemp, pcb->nombreArchivoLQL);
		free(instruccionActualTemp);
		return INSTRUCCION_ERROR;
	}
	return INSTRUCCION_ERROR;
}





static void generar_estadisticas(DynamicAddressingRequest *link){
	if(USAR_SOLO_MEMORIA_PRINCIPAL)
		return;
	if(!link->operacionExitosa)
		return;
	metricas.operacionesTotales += 1;
	switch(link->criterioQueSeUso){
	case SC:
		if(link->tipoOperacion == LECTURA){
			++metricas.At.StrongConsistency.reads;
			metricas.At.StrongConsistency.acumuladorTiemposRead += link->finOperacion - link->inicioOperacion;
			metricas.At.StrongConsistency.readLatency = metricas.At.StrongConsistency.acumuladorTiemposRead / metricas.At.StrongConsistency.reads;
			link->memoria->Metrics.SC.cantidadSelect += 1;
			return;
		}
		if(link->tipoOperacion == ESCRITURA){
			++metricas.At.StrongConsistency.writes;
			metricas.At.StrongConsistency.acumuladorTiemposWrite += link->finOperacion - link->inicioOperacion;
			metricas.At.StrongConsistency.writeLatency = metricas.At.StrongConsistency.acumuladorTiemposWrite / metricas.At.StrongConsistency.writes;
			link->memoria->Metrics.SC.cantidadInsert += 1;
			return;
		}
		break;
	case SHC:
		if(link->tipoOperacion == LECTURA){
			++metricas.At.HashStrongConsistency.reads;
			metricas.At.HashStrongConsistency.acumuladorTiemposRead += link->finOperacion - link->inicioOperacion;
			metricas.At.HashStrongConsistency.readLatency = metricas.At.HashStrongConsistency.acumuladorTiemposRead / metricas.At.HashStrongConsistency.reads;
			link->memoria->Metrics.SHC.cantidadSelect += 1;
			return;
		}
		if(link->tipoOperacion == ESCRITURA){
			++metricas.At.HashStrongConsistency.writes;
			metricas.At.HashStrongConsistency.acumuladorTiemposWrite += link->finOperacion - link->inicioOperacion;
			metricas.At.HashStrongConsistency.writeLatency = metricas.At.HashStrongConsistency.acumuladorTiemposWrite / metricas.At.HashStrongConsistency.writes;
			link->memoria->Metrics.SHC.cantidadInsert += 1;
			return;
		}
		break;
	case EC:
		if(link->tipoOperacion == LECTURA){
			++metricas.At.EventualConsistency.reads;
			metricas.At.EventualConsistency.acumuladorTiemposRead += link->finOperacion - link->inicioOperacion;
			metricas.At.EventualConsistency.readLatency = metricas.At.EventualConsistency.acumuladorTiemposRead / metricas.At.EventualConsistency.reads;
			link->memoria->Metrics.EC.cantidadSelect += 1;
			return;
		}
		if(link->tipoOperacion == ESCRITURA){
			++metricas.At.EventualConsistency.writes;
			metricas.At.EventualConsistency.acumuladorTiemposWrite += link->finOperacion - link->inicioOperacion;
			metricas.At.EventualConsistency.writeLatency = metricas.At.EventualConsistency.acumuladorTiemposWrite / metricas.At.EventualConsistency.writes;
			link->memoria->Metrics.EC.cantidadInsert += 1;
			return;
		}
		break;
	default:
		return;
	}
}





