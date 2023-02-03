#include "file_helpers.h"

static const bool ASSEMBLE_DEBUG = true;

char* load_file_to_string(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror(filename);
        exit(-1);
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

void write_string_to_file(char* filename, char* src) {
    FILE* file = fopen(filename, "wb");
    fputs(src, file);
    fclose(file);
}

// Compile Intel-syntax ASM using NASM and link with gcc
// Example command: nasm -f elf64 -F dwarf -g output.asm && gcc -g -no-pie -o output output.o && rm output.o
void compile_asm(char* asm_src, CompileOptions compile_options) {
    char cmd_str[256];
    snprintf(cmd_str, 255, "%s.asm", compile_options.output_filename);
    write_string_to_file(cmd_str, asm_src);

    // Includes debug symbols
    if (compile_options.debug_annotate_assembly) {
        if (compile_options.link_with_gcc) {
            snprintf(
                cmd_str, 255,
                "nasm -f elf64 -F dwarf -g %1$s.asm -o %1$s.o && gcc -g -no-pie -o %1$s %1$s.o && rm %1$s.o",
                compile_options.output_filename);
            system(cmd_str);
        }
        else {
            snprintf(cmd_str, 255, "nasm -f elf64 -F dwarf -g %1$s.asm -o %1$s",
                     compile_options.output_filename);
            system(cmd_str);
        }
    }
    else {
        if (compile_options.link_with_gcc) {
            snprintf(
                cmd_str, 255,
                "nasm -f elf64 %1$s.asm -o %1$s.o && gcc -no-pie -o %1$s %1$s.o && rm %1$s.o",
                compile_options.output_filename);
            system(cmd_str);
        }
        else {
            snprintf(cmd_str, 255, "nasm -f elf64 %1$s.asm -o %1$s",
                     compile_options.output_filename);
            system(cmd_str);
        }
    }
    if (!compile_options.keep_assembly) {
        snprintf(cmd_str, 255, "rm %1$s.asm", compile_options.output_filename);
        system(cmd_str);
    }
}