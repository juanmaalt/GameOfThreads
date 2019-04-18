#ifndef METADATA_H
#define METADATA_H

typedef struct {
	int blockSize,
	int blocks;
	char* magicNumber;
} Metadata;

#endif // METADATA_H
