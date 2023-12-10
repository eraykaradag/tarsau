#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int isTextFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    int isText = 1; 
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch < 32 && ch != '\n' && ch != '\r' && ch != '\t') {
            isText = 0;
            break;
        }
    }
    fclose(file);
    return isText;  
}

void createHeader(const char* filename,const char* saufile){
	FILE *sectionFile = fopen(saufile,"a+");
	FILE *readFile = fopen(filename,"r");
	if (sectionFile == NULL) {
        perror("Error opening file");
        return -1;
    }
    if (readFile == NULL) {
        perror("Error opening file");
        return -1;
    }
	fseek(readFile,0,SEEK_END);
	long size = ftell(readFile);
	int perms;
	struct stat fileStat;
	if (stat(filename, &fileStat) == 0) {
        perms = fileStat.st_mode & 0777;
    }
    fseek(readFile,0,SEEK_SET);
	fprintf(sectionFile,"%s,%o,%ld|",filename,perms,size);
	fclose(sectionFile);
	fclose(readFile);
}
long getSize(const char* filename){
	FILE *readFile = fopen(filename,"r");
	if (readFile == NULL) {
        perror("Error opening file");
        return -1;
    }
	long size = fseek(readFile,0,SEEK_END);
	fclose(readFile);
	return size;
}
void readAndWrite(const char* filename, const char* saufile){
	FILE *readFile = fopen(filename,"r");
	FILE *sectionFile = fopen(saufile,"a+");
	if (readFile == NULL) {
        perror("Error opening file");
        return -1;
    }
    if (sectionFile == NULL) {
        perror("Error opening file");
        return -1;
    }
	char *line = NULL;
    size_t len = 0;
    ssize_t read;
	while ((read = getline(&line, &len, readFile)) != -1) {
        fputs(line, sectionFile);
    }
    fclose(sectionFile);
	fclose(readFile);
	 if (line != NULL) {
        free(line);
    }
}

char* getHeaderContent(const char* saufile){
	FILE *sectionFile = fopen(saufile,"r");
	if (sectionFile == NULL) {
        perror("Error opening file");
        return -1;
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
	read = getline(&line, &len, readFile);
	fclose(sectionFile);
	return line;
}

