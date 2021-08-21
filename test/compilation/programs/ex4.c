// Interfacing with FILE stuct
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* load_file_to_string(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("The specified source file \"%s\" does not appear to exist\n", filename);
    }

    long size;
    char* buffer;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    // Allocate memory for entire content
    buffer = calloc(1, size + 1);

    // Copy the file contents into the buffer
    fread(buffer, size, 1, file);

    fclose(file);
    return buffer;
}

int main() {
    char* file_content = load_file_to_string("test/compilation/programs/file.txt");
    if (strcmp(file_content, "Successful file loading! ") == 0) {
        return 1;
    }
    return 2;
}