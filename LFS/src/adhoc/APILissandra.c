/*
 * APILissandra.c
 *
 *  Created on: 20 may. 2019
 *      Author: juanmaalt
 */

#include "APILissandra.h"



/*INICIO FUNCIONES API*/

Operacion selectAPI(Comando comando){
	/*Creo variable resultado del tipo Operacion para devolver el mensaje*/
	Operacion resultadoSelect;
	resultadoSelect.TipoDeMensaje = ERROR;
	/*Checkea existencia de la Memtable*/
	if (!memtable) {
		log_error(logger_invisible, "No existe una memtable creada, no se puede realizar la operación.");
		resultadoSelect.Argumentos.ERROR.mensajeError = string_from_format("No existe una memtable creada, no se puede realizar la operación.");
		return resultadoSelect;
	}
	/*Checkea existencia de la tabla solicitada*/
	if(!(existeTabla(comando.argumentos.SELECT.nombreTabla))){
		log_error(logger_invisible, "No existe una tabla asociada a la key solicitada.");
		resultadoSelect.Argumentos.ERROR.mensajeError = string_from_format("No existe una tabla asociada a la key solicitada.");
		return resultadoSelect;
	}
	/*Levanta la metadata de la tabla*/
	t_config* metadataFile = leerMetadata(comando.argumentos.SELECT.nombreTabla);
	if(getMetadata(comando.argumentos.SELECT.nombreTabla, metadataFile)==EXIT_FAILURE){
		resultadoSelect.Argumentos.ERROR.mensajeError = string_from_format("No existe el archivo Metadata de la tabla solicitada.");
		return resultadoSelect;
	}

	/*Levanta la lista de registros relacionados a la tabla*/
	t_list* data = getData(comando.argumentos.SELECT.nombreTabla);

	/*Calculo el nro de partición en la que se encuentra la key*/
	int particionNbr = calcularParticionNbr(comando.argumentos.SELECT.key, metadata.partitions);
	printf("Partición Nro: %d\n", particionNbr);

	/*Creo la lista de valores que condicen con dicha key*/
	t_list* listaDeValues = list_create();

	/*Busco en Memtable*/
	listaDeValues=buscarValueEnLista(data, comando.argumentos.SELECT.key);

	/*Busco en Temporales*/
	leerTemps(comando.argumentos.SELECT.nombreTabla, comando.argumentos.SELECT.key, listaDeValues);

	/*Busco en Bloques*/
	char* listaDeBloques= obtenerListaDeBloques(particionNbr, comando.argumentos.SELECT.nombreTabla);
	list_add(listaDeValues, fseekBloque(atoi(comando.argumentos.SELECT.key), listaDeBloques));

	/*Recorro la tabla y obtengo el valor más reciente*/
	//recorrerTabla(listaDeValues);//Función ad-hoc para testing
	getValueMasReciente(listaDeValues, &resultadoSelect);

	/*Libero recursos en memoria*/
	list_destroy(listaDeValues);
	config_destroy(metadataFile);

	return resultadoSelect;
}


Operacion insertAPI(Comando comando){
	/*Creo variable resultado del tipo Operacion para devolver el mensaje*/
	Operacion resultadoInsert;
	resultadoInsert.TipoDeMensaje = ERROR;

	/*Checkea existencia de la Memtable*/
	if (!memtable) {
		log_error(logger_invisible, "No existe una memtable creada, no se puede realizar la operación.");
		resultadoInsert.Argumentos.ERROR.mensajeError = string_from_format("No existe una memtable creada, no se puede realizar la operación.");
		return resultadoInsert;
	}
	/*Checkea existencia de la tabla solicitada*/
	if(!(existeTabla(comando.argumentos.INSERT.nombreTabla))){
		log_error(logger_invisible, "No existe una tabla asociada a la key solicitada.");
		resultadoInsert.Argumentos.ERROR.mensajeError = string_from_format("No existe la tabla solicitada.");
		return resultadoInsert;
	}

	/*Levanta la metadata de la tabla*/
	t_config* metadataFile = leerMetadata(comando.argumentos.INSERT.nombreTabla);
	if(getMetadata(comando.argumentos.SELECT.nombreTabla, metadataFile)==EXIT_FAILURE){
		resultadoInsert.Argumentos.ERROR.mensajeError = string_from_format("No existe el archivo Metadata de la tabla solicitada.");
		return resultadoInsert;
	}

	//checkExisteMemoria(); //Verificar si existe en memoria una lista de datos a dumpear. De no existir, alocar dicha memoria.

	/*Reservo espacio y aloco los datos a insertar*/
	Registro* reg = malloc(sizeof(Registro));
	reg->key = atoi(comando.argumentos.INSERT.key);
	reg->value = string_from_format(comando.argumentos.INSERT.value);
	reg->timestamp=checkTimestamp(comando.argumentos.INSERT.timestamp);

	/*Obtengo la lista de registros a partir de la tabla solicitada*/
	t_list* data = getData(comando.argumentos.INSERT.nombreTabla);

	/*Agrego el registro a dicha lista*/
	list_add(data, reg);

	//INICIO AD-HOC//
	/*
	char* path = malloc(100 * sizeof(char));
	setPathTabla(path, comando.argumentos.INSERT.nombreTabla);
	insertInFile(path, particionNbr, comando.argumentos.INSERT.key, comando.argumentos.INSERT.value); //TODO: Borrar?
	//FIN AD-HOC//
	*/

	//printf("Registro->Timestamp= %llu\n", reg->timestamp);
	//printf("Registro->Key= %d\n", reg->key);
	//printf("Registro->Value= %s\n", reg->value);

	/*Loggeo el INSERT exitoso y le aviso a la Memoria*/
	log_info(logger_invisible, "INSERT realizado con éxito.");
	resultadoInsert.TipoDeMensaje = TEXTO_PLANO;
	resultadoInsert.Argumentos.TEXTO_PLANO.texto = string_from_format("INSERT realizado con éxito.");
	/*Libero recursos*/
	config_destroy(metadataFile);

	return resultadoInsert;
}


Operacion createAPI(Comando comando){
	/*Creo variable resultado del tipo Operacion para devolver el mensaje*/
	Operacion resultadoCreate;
	resultadoCreate.TipoDeMensaje = ERROR;

	/*Checkeo la existencia de la tabla. De existir la misma loggeo un error*/
	if(existeTabla(comando.argumentos.CREATE.nombreTabla)){
		log_error(logger_invisible, "La tabla solicitada ya existe en el sistema.");
		resultadoCreate.Argumentos.ERROR.mensajeError = string_from_format("La tabla solicitada ya existe en el sistema.");
		return resultadoCreate;
	}

	/*Obtengo la ruta del directorio donde van a estar los archivos de la tabla*/
	char* path = string_from_format("%sTables/%s", config.punto_montaje, comando.argumentos.CREATE.nombreTabla);

	/*Creo directorio para la tabla solicitada*/
	crearDirectorioTabla(path);

	/*Creo el archivo de metadata de la tabla*/
	string_append(&path, "/");
	crearArchivo(path, "Metadata");

	/*Escribo la metadata*/
	escribirArchivoMetadata(path, comando);

	/*Creo los archivos binarios vacíos*/
	crearArchivosBinarios(path, atoi(comando.argumentos.CREATE.numeroParticiones));

	/*Creo la Tabla en la Memtable*/
	crearTablaEnMemtable(comando.argumentos.CREATE.nombreTabla);

	/*Inicio el proceso de compactación*/
	/*TODO:arreglar
	char* nombreTabla = string_from_format(comando.argumentos.CREATE.nombreTabla);
	if(iniciarCompactacion(nombreTabla) == EXIT_FAILURE){
		log_error(logger_error,"APILissandra.c: <CREATE> No se pudo iniciar el hilo de compactación");
		return resultadoCreate;
	}
	*/

	/*Loggeo el CREATE exitoso y le aviso a la Memoria*/
	log_info(logger_invisible, "CREATE realizado con éxito.");
	resultadoCreate.TipoDeMensaje = TEXTO_PLANO;
	resultadoCreate.Argumentos.TEXTO_PLANO.texto = string_from_format("CREATE realizado con éxito.");

	return resultadoCreate;
}


Operacion describeAPI(Comando comando){
	/*Creo variable resultado del tipo Operacion para devolver el mensaje*/
	Operacion resultadoDescribe;
	resultadoDescribe.TipoDeMensaje = DESCRIBE_REQUEST;

	/*Reservo espacio para los paths*/
	char* path = string_from_format("%sTables/", config.punto_montaje);
	//char* pathMetadata = malloc(1000 * sizeof(char));

	char* string=string_new();//TODO: esto podría traer condición de carrera

	getStringDescribe(path, string, comando.argumentos.DESCRIBE.nombreTabla, &resultadoDescribe);
	//printf("Describe_string: %s\n", resultadoDescribe.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);

	/*Loggeo el CREATE exitoso y le aviso a la Memoria*/
	log_info(logger_invisible, "DESCRIBE realizado con éxito.");
	log_info(logger_invisible, "DESCRIBE: %s", resultadoDescribe.Argumentos.DESCRIBE_REQUEST.resultado_comprimido);

	return resultadoDescribe;
}


Operacion dropAPI(Comando comando){
	/*Creo variable resultado del tipo Operacion para devolver el mensaje*/
	Operacion resultadoDrop;
	resultadoDrop.TipoDeMensaje = ERROR;

	if(existeTabla(comando.argumentos.DROP.nombreTabla)){
		/*Borro la entrada de la memtable*/
		dictionary_remove(memtable, comando.argumentos.DROP.nombreTabla);
	}
		limpiarBloquesEnBitarray(comando.argumentos.DROP.nombreTabla);

		/*Reservo espacio para los paths*/
		char* pathFolder = string_from_format("%sTables/%s", config.punto_montaje, comando.argumentos.DROP.nombreTabla);

		int removido=removerDirectorio(pathFolder);
		//printf("resultadoDrop= %d\n", removido);

		resultadoDrop.TipoDeMensaje = TEXTO_PLANO;
		resultadoDrop.Argumentos.TEXTO_PLANO.texto = string_from_format("DROP realizado con exito.");

	if(removido!=0){
		resultadoDrop.Argumentos.ERROR.mensajeError = string_from_format("No existe la tabla que intenta Borrar");
	}

	return resultadoDrop;
}
/*FIN FUNCIONES API*/


