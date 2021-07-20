#pragma once
#include <stdio.h>
#include <stdlib.h>

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

void write_string_to_file(char* filename, char *src) {
    FILE *file = fopen(filename, "wb");
    fputs(src, file);
    fclose(file);
}

// Compile Intel-syntax ASM using NASM and link with gcc
void compile_asm(char *asm_src) {
    write_string_to_file("output.asm", asm_src);
    system("nasm -f elf64 output.asm && gcc -no-pie -o output output.o && rm output.o");
}