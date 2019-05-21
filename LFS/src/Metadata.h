#ifndef METADATA_H
#define METADATA_H

typedef struct {
	char* consistency;
	int partitions;
	int compaction_time;
}Metadata_tabla;

typedef struct {
	int blockSize;
	int blocks;
	char* magicNumber;
}Metadata_FS;

#endif // METADATA_H
