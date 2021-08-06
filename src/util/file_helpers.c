#include "file_helpers.h"

char* load_file_to_string(char* filename) {
    FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("The specified source file \"%s\" does not appear to exist\n", filename);
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
    // Includes debug symbols
    system("nasm -f elf64 -F dwarf -g output.asm && gcc -g -no-pie -o output output.o && rm output.o");
}

char* isolate_file_dir(char* filepath) {
    int index = str_index_of_reverse(filepath, '/');
    return str_substr(filepath, index+1);
}