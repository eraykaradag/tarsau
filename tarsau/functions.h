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

int createHeader(const char *filename, const char *archive_file_name) {
    struct stat fileStat;
    if (stat(filename, &fileStat) != 0) {
        perror("Error getting file stats");
        return -1;
    }
    int perms = fileStat.st_mode & 0777;
    long size = fileStat.st_size;

    FILE *archive_file = fopen(archive_file_name, "a+");
    if (archive_file == NULL) {
        perror("Error opening archive file");
        return -1;
    }

    fprintf(archive_file, "%s,%o,%ld|", filename, perms, size);
    fclose(archive_file);

    return 0;
}


long getSize(const char *filename) {
    FILE *readFile = fopen(filename, "r");
    if (readFile == NULL) {
        perror("Error opening file");
        return -1;
    }
    long size = fseek(readFile, 0, SEEK_END);
    fclose(readFile);
    return size;
}

int readAndWrite(const char *file_to_be_archived_name, const char *archive_file_name) {
    FILE *file_to_be_archived = fopen(file_to_be_archived_name, "r");
    if (file_to_be_archived == NULL) {
        perror("Error opening source file");
        return -1;
    }

    FILE *archive_file = fopen(archive_file_name, "a+");
    if (archive_file == NULL) {
        perror("Error opening archive file");
        fclose(file_to_be_archived);
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file_to_be_archived)) != -1) {
        fputs(line, archive_file);
    }

    free(line);
    fclose(archive_file);
    fclose(file_to_be_archived);

    return 0;
}

int endsWith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len)
        return 0;
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}


//char* getHeaderContent(const char* saufile){
//	FILE *sectionFile = fopen(saufile,"r");
//	if (sectionFile == NULL) {
//        perror("Error opening file");
//        return -1;
//    }
//    char *line = NULL;
//    size_t len = 0;
//    ssize_t read;
//	read = getline(&line, &len, readFile);
//	fclose(sectionFile);
//	return line;
//}

