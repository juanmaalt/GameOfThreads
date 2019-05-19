#include "Memtable.h"

char* select(Memtable* memtable, char* nombreTabla, char* key) {
	Node* selectedNode = NULL;
	Node* node = memtable->first;
	while (node) {
		if (nodeMatch(node, nombreTabla, key)) {
			selectedNode = bestMatch(selectedNode, node);
		}
		node = node->next;
	}
	char* selected = NULL;
	if (selectedNode) {
		selected = selectedNode->value;
	}
	return selected;
}

Node* bestMatch(Node* selectedNode, Node* node) {
	if (!selectedNode || strcmp(selectedNode->timestamp, node->timestamp) > 0) {
		return node;
	}
	return selectedNode;
}

int nodeMatch(Node* node, char* nombreTabla, char* key) {
	return strcmp(node->nombreTabla, nombreTabla) == 0 && strcmp(node->key, key) == 0;
}

void insert(Memtable* memtable, char* nombreTabla, char* key, char* timestamp, char* value) {
	Node* newNode = createNode(nombreTabla, key, timestamp, value);
	if (!memtable->first) {
		memtable->first = newNode;
	} else {
		Node* last = lastNode(memtable);
		last->next = newNode;
	}
}

Memtable* createMemtable() {
	Memtable* memtable = malloc(sizeof(Memtable));
	memtable->first = NULL;
	return memtable;
}

Node* createNode(char* nombreTabla, char* key, char* timestamp, char* value) {
	Node* newNode = malloc(sizeof(Node));
	newNode->nombreTabla = nombreTabla;
	newNode->key = key;
	newNode->timestamp = timestamp;
	newNode->value = value;
	newNode->next = NULL;
	return newNode;
}

Node* lastNode(Memtable* memtable) {
	Node* node = memtable->first;
	while(node->next) {
		node = node->next;
	}
	return node;
}

void destroyNode(Node* temp) {
	free(temp->nombreTabla);
	free(temp->key);
	free(temp->timestamp);
	free(temp->value);
	free(temp);
}

void destroyTable(Memtable* memtable) {
	Node* node = memtable->first;
	while(node->next) {
		Node* temp = node;
		node = temp->next;
		destroyNode(temp);
	}
	destroy(node);
	free(memtable);
}
