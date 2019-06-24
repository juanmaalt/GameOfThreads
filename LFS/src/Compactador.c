
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
