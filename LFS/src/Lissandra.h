#ifndef LISSANDRA_H
#define LISSANDRA_H

#include "./Metadata.h"
#include "./Memtable.h"

char* select(char* nombreTable, int key);

void insert(char* nombreTable, int key, char* value, long timestamp);

void create(char* nombreTable, char* value, int particiones, long compactionTime);

Metadata** describe(void);

Metadata* describe(char* nombreTabla);

void drop(char* nombreTabla);

#endif // LISSANDRA_H
