#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <ctype.h>

#define HEADER_SIZE 10

enum Mode{
    NONE,
    ARCHIVING,
    EXTRACTING
};

typedef struct {
    char *file_name;
    mode_t permissions;
    long size;
} FileHeader;


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
    unsigned int perms = fileStat.st_mode & 0777;
    long size = fileStat.st_size;

    // Open the archive file in "r+" mode to allow read and write
    FILE *archive_file = fopen(archive_file_name, "r+");
    if (archive_file == NULL) {
        // If opening in "r+" fails (probably because the file doesn't exist), try "w" to create a new file
        archive_file = fopen(archive_file_name, "w");
        if (archive_file == NULL) {
            perror("Error opening archive file");
            return -1;
        }
    }

    size_t header_size = 0;
    fprintf(archive_file, "%010ld|", header_size);
    fseek(archive_file, 0, SEEK_END);
    fprintf(archive_file, "%s,%o,%ld|", filename, perms, size);
    header_size = ftell(archive_file);
    fseek(archive_file, 0, SEEK_SET);
    fprintf(archive_file, "%010ld|", header_size);
    fflush(archive_file);
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
    while (getline(&line, &len, file_to_be_archived) != -1) {
        fputs(line, archive_file);
    }

    free(line);
    fclose(archive_file);
    fclose(file_to_be_archived);

    return 0;
}

int endsWith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return -1;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len)
        return -1;
    int compare_strings = strncmp(str + str_len - suffix_len, suffix, suffix_len);
    return compare_strings == 0 ? 0 : compare_strings;
}

void truncateFile(char *file_name){
    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        perror("Error: opening file to truncate");
    }
    fclose(file);
}

int validateHeaderFormat(char *header) {
    // Check if the first 10 characters are digits
    for (int i = 0; i < HEADER_SIZE; i++) {
        if (!isdigit(header[i])) {
            fprintf(stderr, "Invalid header format: first 10 bytes should represent the total "
                            "header length and must be numeric.\n");
            return -1;
        }
    }

    // Convert the first 10 characters to a number
    char length_str[HEADER_SIZE + 1] = {0};
    strncpy(length_str, header, HEADER_SIZE);
    long header_length = strtol(length_str, NULL, 10);

    // Check for the total length
    if (header_length != strlen(header)) {
        fprintf(stderr, "Invalid header format: header length mismatch with the actual header size.\n");
        return -1;
    }

    // Validate each file entry
    const char *entry_start = header + HEADER_SIZE + 1; // Skip the header size and delimiter
    const char *entry_end = strchr(entry_start, '|');

    while (entry_end != NULL) {
        int comma_count = 0;
        for (const char *p = entry_start; p < entry_end; p++) {
            if (*p == ',') comma_count++;
        }
        if (comma_count != 2) {
            fprintf(stderr, "Invalid header format: incorrect file entry format\n");
            return -1;
        }

        // Move to the next entry
        entry_start = entry_end + 1;
        entry_end = strchr(entry_start, '|');
    }

    return 0;
}


int getHeaderContent(FileHeader **headers, int *num_headers, const char *archive_file_name) {
    FILE *archive_file = fopen(archive_file_name, "r");
    if (archive_file == NULL) {
        perror("Error opening archive file");
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, archive_file) == -1) {
        perror("Error reading header");
        fclose(archive_file);
        return -1;
    }

    if (line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
    }

    // Validate the header format
    if (validateHeaderFormat(line) != 0) {
        fclose(archive_file);
        free(line);
        return -1;
    }

    char *header_part = line + HEADER_SIZE + 1;
    char *saveptr;
    char *token = strtok_r(header_part, "|", &saveptr);
    int idx = 0;

    while (token) {
        if (strlen(token) == 0) {
            token = strtok_r(NULL, "|", &saveptr);
            continue;
        }

        *headers = realloc(*headers, (idx + 1) * sizeof(FileHeader));
        if (*headers == NULL) {
            perror("Memory allocation failed for headers");
            fclose(archive_file);
            free(line);
            return -1;
        }

        FileHeader *header = &((*headers)[idx]);
        header->file_name = NULL;
        sscanf(token, "%m[^,],%o,%ld", &header->file_name, &header->permissions, &header->size);
        if (header->file_name == NULL) {
            // Clean up
            for (int i = 0; i < idx; i++) {
                free((*headers)[i].file_name);
            }
            free(*headers);
            fclose(archive_file);
            free(line);
            return -1;
        }

        idx++;
        token = strtok_r(NULL, "|", &saveptr);
    }

    *num_headers = idx;
    fclose(archive_file);
    free(line);

    return 0;
}

int getHeaderSize(const char* archive_file_name){
    FILE *archive_file = fopen(archive_file_name,"r");
    if (archive_file == NULL) {
        perror("Error opening file");
        fclose(archive_file);
        return -1;
    }
    char header_size_str[11] = {0}; // first 10 byte + 1 for null terminator
    size_t result = fread(header_size_str,1, HEADER_SIZE, archive_file);
    fclose(archive_file);
    if (result != 10){
        perror("Error reading header size\n");
        fclose(archive_file);
        return -1;
    }
    char *endptr;
    errno = 0;
    long val = strtol(header_size_str, &endptr, 10);

    if (errno != 0) {
        perror("strtol");
        return -1;
    }
    if (endptr == header_size_str) {
        fprintf(stderr, "No digits were found\n");
        return -1;
    }

    if (val < INT_MIN || val > INT_MAX) {
        fprintf(stderr, "Header size out of int range\n");
        return -1;
    }
    return (int)val;
}

int createDirectoryStructure(const char* path) {
    char* temp_path = strdup(path);
    if (temp_path == NULL) {
        perror("Failed to duplicate path");
        return -1;
    }

    // dirname can modify the input string, so we use a copy
    char* dir_path = dirname(temp_path);

    // Check if the directory already exists
    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        // Create the directory if it doesn't exist
        if (mkdir(dir_path, 0777) != 0) {
            perror("Failed to create directory");
            free(temp_path);
            return -1;
        }
    }

    free(temp_path);
    return 0;
}

int extractFiles(const char* archive_file_name, const char* extract_directory) {
    FileHeader *headers = NULL;
    int num_headers = 0;

    // Get header size from archive file
    int header_size = getHeaderSize(archive_file_name);
    if (header_size < 0) {
        return -1; // Error message already printed by getHeaderSize
    }

    // Get header content from archive file
    if (getHeaderContent(&headers, &num_headers, archive_file_name) != 0) {
        return -1; // Error message already printed by getHeaderContent
    }

    for (int i = 0; i < num_headers; ++i) {
        char* f = headers[i].file_name;
        mode_t p = headers[i].permissions;
        long s = headers[i].size;
    }

    // Open the archive file for reading file contents
    FILE *archive_file = fopen(archive_file_name, "r");
    if (archive_file == NULL) {
        perror("Error opening archive file");
        free(headers);
        return -1;
    }

    // Skip the header section in the archive file
    fseek(archive_file, header_size + 1, SEEK_SET);

    for (int i = 0; i < num_headers; i++) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", extract_directory, headers[i].file_name);

        // Create directory structure for the file
        if (createDirectoryStructure(full_path) != 0) {
            fclose(archive_file);
            free(headers);
            return -1;
        }

        // Open file for writing in text mode
        FILE *out_file = fopen(full_path, "w");
        if (out_file == NULL) {
            perror("Error opening output file");
            fclose(archive_file);
            free(headers);
            return -1;
        }

        // Read and write the file content
        char buffer[1024];
        long bytes_to_read = headers[i].size;
        while (bytes_to_read > 0) {
            size_t bytes_to_read_now = (bytes_to_read < sizeof(buffer)) ? bytes_to_read : sizeof(buffer);
            size_t bytes_read = fread(buffer, 1, bytes_to_read_now, archive_file);

            if (bytes_read == 0) {
                break;
            }

            fwrite(buffer, 1, bytes_read, out_file);
            bytes_to_read -= bytes_read;
        }

        fclose(out_file);

        // Set the file permissions to match the header
        if (chmod(full_path, headers[i].permissions) == -1) {
            perror("Error setting file permissions");
            fclose(archive_file);
            free(headers);
            return -1;
        }
    }

    fclose(archive_file);

    // Clean up allocated headers
    for (int i = 0; i < num_headers; i++) {
        free(headers[i].file_name);
    }
    free(headers);

    return 0;
}

