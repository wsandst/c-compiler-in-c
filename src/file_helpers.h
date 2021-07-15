#pragma once
#include <stdio.h>

char* load_file_to_string(char* filename) {
    FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("The specified source file does not exist\n");
        exit(1);
	}

    long size;
    char *buffer;

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    rewind(file);

    // Allocate memory for entire content
    buffer = calloc(1, size+1);

    // Copy the file contents into the buffer
    fread(buffer, size, 1, file);

    fclose(file);
    return buffer;
}