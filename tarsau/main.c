#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "functions.h"

int main(int argc, char **argv) {
    // current working directory for test purposes, clear later.
//    char cwd[1024];
//    if (getcwd(cwd, sizeof(cwd)) != NULL) {
//        printf("Current working dir: %s\n", cwd);
//    } else {
//        perror("getcwd() error");
//    }

    if (argc < 2) {
        printf("Error: improper command!\n");
        return -1;
    }

    char *archive_file_name = "a.sau";
    bool is_archiving = false;
    bool o_opt = false;
    int num_text_files = 0;
    int totalSize = 0;

    for (int i = 1; i < argc; i++) {
//        char *argument = argv[i];
//        printf("%s\n", argument);
        if (strcmp(argv[i], "-b") == 0) {
            if (is_archiving) {
                printf("Error: Multiple -b commands are not allowed.\n");
                return -1;
            }
            is_archiving = true;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0) {
            if (o_opt) {
                printf("Error: Multiple -o commands are not allowed.\n");
                return -1;
            }
            if (i + 1 < argc) {
                // Check if the file already ends with '.sau' if so do not add another '.sau'.
                if ((endsWith(archive_file_name, ".sau") != 0)) {

                    archive_file_name = malloc(strlen(argv[i + 1]) +
                                               5); // I'm allocating new memory for filename + ".sau" + the null terminator
                    if (!archive_file_name) {
                        perror("Memory allocation failed");
                        return -1;
                    }
                    sprintf(archive_file_name, "%s.sau", argv[++i]);
                }
                o_opt = true;
            } else {
                printf("Error: Missing filename after -o option.\n");
                return -1;
            }
        }
    }

    if (is_archiving) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-b") != 0 && strcmp(argv[i], "-o") != 0 && strcmp(argv[i - 1], "-o") != 0) {
                if (isTextFile(argv[i]) == 0) {
                    printf("Error: %s input file format is incompatible! \n", argv[i]);
                    return -1;
                }
                num_text_files++;
                totalSize += getSize(argv[i]);
            }
        }
        if (num_text_files > 32) {
            printf("Error: This archiver can only archive 32 files at a time. Please reduce the number of files.\n");
            return -1;
        }
        if (totalSize > (200 * 1024 * 1024)) {
            printf("Error: The data that you want to archive is larger than 200MiB\n");
            return -1;
        }
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-b") != 0 && strcmp(argv[i], "-o") != 0 && strcmp(argv[i - 1], "-o") != 0) {
                createHeader(argv[i], archive_file_name);
            }
        }

        FILE *file = fopen(archive_file_name, "a+");
        fprintf(file, "\n");
        fclose(file); //spacing after header info

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-b") != 0 && strcmp(argv[i], "-o") != 0 && strcmp(argv[i - 1], "-o") != 0) {
                readAndWrite(argv[i], archive_file_name); //write data into .sau file
            }
        }
    } else {
        printf("Error: There is no such an option in tarsau command! --> -b for archiving -a for extracting\n");
        return -1;
    }
    free(archive_file_name);
    printf("SUCCESS! WOOHOOO!\n");
    return 0;
}
