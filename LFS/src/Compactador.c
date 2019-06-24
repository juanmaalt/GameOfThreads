
void compactarTabla(char* nombreTabla) {
	char* path = malloc(100 * sizeof(char));
	setPathTabla(path, nombreTabla);
	DIR* dir = openddir(path);
	struct dirent *de;

	while ((de = readdir(dir)) != NULL) {
		if (isTmp(de->d_name)) {
			FILE* file = fopen(de->d_name);
			compactarArchivo(file)
			fclose(file);
			deleteFile(de->d_name);
		}
	}
}

int isTmp(char* name) {
	return strcmp(extension(name), "tmp") == 0;
}

char* extension(char *filename) {
	char* dot = strrchr(filename, '.');
	if (!dot || dot == filename) {
		return "";
	}
	return dot + 1;
}

void compactarArchivo(FILE* file) {
	char* line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stream) != -1) {
		Registro* registro = registro(line, len);
		compactarRegistro(registro);
		free(registro);
	}
}
