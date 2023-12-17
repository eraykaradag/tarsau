#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "functions.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: improper command!\n");
        return -1;
    }

    char* archive_file_name = strdup("a.sau");
    if (archive_file_name == NULL) {
        free(archive_file_name);
        perror("Memory allocation failed for archive file name.");
        return -1;
    }
    char* extract_file_name = NULL;
    char* extract_directory = NULL;
    enum Mode MODE = NONE;
    bool o_opt = false;
    int num_text_files = 0;
    long totalSize = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0) {
            if (MODE == ARCHIVING) {
                printf("Error: Multiple -b commands are not allowed.\n");
                return -1;
            }
            if (i + 1 < argc) {
                if (argv[i + 1][0] == '-') {
                    printf("Error: Missing filename after -b option.\n");
                    return -1;
                }
                MODE = ARCHIVING;
                continue;
            } else {
                printf("Error: Missing filename after -b option.\n");
                return -1;
            }
        }
        if (strcmp(argv[i], "-o") == 0) {
            if (o_opt) {
                printf("Error: Multiple -o commands are not allowed.\n");
                return -1;
            }
            if (i + 1 < argc) {
                o_opt = true;
                if (argv[i + 1][0] == '-') continue;
                free(archive_file_name);
                archive_file_name = NULL;
                archive_file_name = strdup(argv[i + 1]);
                if (archive_file_name == NULL) {
                    free(archive_file_name);
                    perror("Memory allocation failed for archive file name.");
                    return -1;
                }
                // Check if the file already ends with '.sau' if so do not add another '.sau'.
                if ((endsWith(archive_file_name, ".sau") != 0)) {
                    char* temp = realloc(archive_file_name, strlen(argv[i + 1]) + strlen(".sau") + 1);
                    if (!temp) {
                        free(archive_file_name);
                        perror("Memory reallocation failed for archive file name.");
                        return -1;
                    }
                    archive_file_name = temp;

                    // Concatenate ".sau" to the filename
                    sprintf(archive_file_name, "%s.sau", argv[i + 1]);
                }
            }
        }
        if (strcmp(argv[i], "-a") == 0) {
            if (archive_file_name == NULL) {
                free(archive_file_name);
                archive_file_name = NULL;
            }
            if (MODE == ARCHIVING || o_opt) {
                free(archive_file_name);
                printf("Error: -a option cannot be used with -b or -o options.\n");
                return -1;
            }
            MODE = EXTRACTING;
            if (i + 1 < argc && endsWith(argv[i + 1], ".sau") == 0) {
                extract_file_name = strdup(argv[++i]);
                if (extract_file_name == NULL) {
                    free(archive_file_name);
                    free(extract_file_name);
                    perror("Memory allocation failed for extract file name.");
                    return -1;
                }
            } else {
                if (i + 1 > argc)
                    printf("Error: Missing archive file name after -a option.\n");
                else
                    printf("Error: Archive file is inappropriate or corrupt!\n");
                return -1;
            }
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                extract_directory = strdup(argv[++i]);
                if (extract_directory == NULL) {
                    free(extract_directory);
                    free(archive_file_name);
                    perror("Memory allocation failed for extract directory.");
                    return -1;
                }
            }
            break;
        }
    }

    if (MODE == ARCHIVING) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-b") != 0 && strcmp(argv[i], "-o") != 0 && strcmp(argv[i - 1], "-o") != 0) {
                if (isTextFile(argv[i]) == 0) {
                    printf("Error: %s input file format is incompatible! \n", argv[i]);
                    free(archive_file_name);
                    return -1;
                }
                num_text_files++;
                totalSize += getSize(argv[i]);
            }
        }
        if (num_text_files > 32) {
            printf("Error: This archiver can only archive 32 files at a time. Please reduce the number of files.\n");
            free(archive_file_name);
            return -1;
        }
        if (totalSize > (200 * 1024 * 1024)) {
            printf("Error: The data that you want to archive is larger than 200MiB\n");
            free(archive_file_name);
            return -1;
        }

        truncateFile(archive_file_name);

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-b") != 0 && strcmp(argv[i], "-o") != 0 && strcmp(argv[i - 1], "-o") != 0) {
                createHeader(argv[i], archive_file_name);
            }
        }

        FILE* file = fopen(archive_file_name, "a+");
        fprintf(file, "\n");
        fclose(file); //spacing after header info

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-b") != 0 && strcmp(argv[i], "-o") != 0 && strcmp(argv[i - 1], "-o") != 0) {
                readAndWrite(argv[i], archive_file_name); //write data into .sau file
            }
        }
    } else if (MODE == EXTRACTING) {
        if (extract_directory == NULL) {
            extract_directory = strdup("."); // current directory by default
            if (extract_directory == NULL) {
                free(extract_directory);
                free(archive_file_name);
                perror("Memory allocation failed for extract directory.");
                return -1;
            }
        }
        int result = extractFiles(archive_file_name, extract_directory);
        if (result != 0) {
            free(archive_file_name);
            free(extract_directory);
            fprintf(stderr, "Error: Extracting was not successful.\n");
            return -1;
        }


    } else {
        printf("Error: There is no such an option in tarsau command! --> -b for archiving -a for extracting.\n");
        return -1;
    }
    free(archive_file_name);
    printf("SUCCESS! WOOHOOO!\n");
    return 0;
}
