/*
 * APIMemoria.c
 *
 *  Created on: 15 may. 2019
 *      Author: fdalmaup
 */

#include "APIMemoria.h"


/*
* La operación Select permite la obtención del valor de una key dentro de una tabla. Para esto, se utiliza la siguiente nomenclatura:
SELECT [NOMBRE_TABLA] [KEY]
Ej:
SELECT TABLA1 3

Esta operación incluye los siguientes pasos:
1. Verifica si existe el segmento de la tabla solicitada y busca en las páginas del mismo si contiene key solicitada.
Si la contiene, devuelve su valor y finaliza el proceso.
2. Si no la contiene, envía la solicitud a FileSystem para obtener el valor solicitado y almacenarlo.
3. Una vez obtenido se debe solicitar una nueva página libre para almacenar el mismo.
En caso de no disponer de una página libre, se debe ejecutar el algoritmo de reemplazo y,
en caso de no poder efectuarlo por estar la memoria full, ejecutar el Journal de la memoria.
*
* */
void selectAPI(char* input, Comando comando){
	printf("La linea recibida es: %s\n",input);
}

/*
			 * La operación Insert permite la creación y/o actualización de una key dentro de una tabla. Para esto, se utiliza la siguiente nomenclatura:
INSERT [NOMBRE_TABLA] [KEY] “[VALUE]”
Ej:
INSERT TABLA1 3 “Mi nombre es Lissandra”

Esta operación incluye los siguientes pasos:
1. Verifica si existe el segmento de la tabla en la memoria principal.
De existir, busca en sus páginas si contiene la key solicitada y de contenerla actualiza el valor insertando el Timestamp actual.
En caso que no contenga la Key, se solicita una nueva página para almacenar la misma.
Se deberá tener en cuenta que si no se disponen de páginas libres
aplicar el algoritmo de reemplazo (LRU) y en caso de que la memoria se encuentre full iniciar el proceso Journal.
2. En caso que no se encuentre el segmento en memoria principal, se creará y se agregará la nueva Key con el Timestamp actual,
junto con el nombre de la tabla en el segmento. Para esto se debe generar el nuevo segmento y solicitar una nueva página
(aplicando para este último la misma lógica que el punto anterior).

Cabe destacar que la memoria no verifica la existencia de las tablas al momento de insertar nuevos valores. De esta forma,
no tiene la necesidad de guardar la metadata del sistema en alguna estructura administrativa. En el caso que al momento de realizar el Journaling
una tabla no exista, deberá informar por archivo de log esta situación, pero el proceso deberá actualizar correctamente las tablas que sí existen.

Cabe aclarar que esta operatoria en ningún momento hace una solicitud directa al FileSystem, la misma deberá manejarse siempre
dentro de la memoria principal.
			 * */

void insertAPI(char* input, Comando comando){
	printf("La linea recibida es: %s\n",input);
}
