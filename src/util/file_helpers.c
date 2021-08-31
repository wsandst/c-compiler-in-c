#include "file_helpers.h"

static const bool ASSEMBLE_DEBUG = true;

char* load_file_to_string(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Source file loading error");
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
    if (ASSEMBLE_DEBUG) {
        snprintf(cmd_str, 255, "nasm -f elf64 -F dwarf -g %1$s.asm",
                 compile_options.output_filename);
        system(cmd_str);
        if (compile_options.link_with_gcc) {
            snprintf(cmd_str, 255, "gcc -g -no-pie -o %1$s %1$s.o && rm %1$s.o",
                     compile_options.output_filename);
        }
        system(cmd_str);
    }
    else {
        snprintf(cmd_str, 255, "nasm -f elf64 %1$s.asm", compile_options.output_filename);
        system(cmd_str);
        if (compile_options.link_with_gcc) {
            snprintf(cmd_str, 255, "gcc -no-pie -o %1$s %1$s.o && rm %1$s.o",
                     compile_options.output_filename);
        }
        system(cmd_str);
    }
}

char* isolate_file_dir(char* filepath) {
    int index = str_index_of_reverse(filepath, '/');
    return str_substr(filepath, index + 1);
}

char* isolate_file_from_path(char* filepath) {
    int index = str_index_of_reverse(filepath, '/');
    return str_copy(filepath + index + 1);
}

// Parse compiler command line options
CompileOptions parse_compiler_options(int argc, char** argv) {
    CompileOptions options;
    options.link_with_gcc = true;
    if (argc > 1) {
        if (strcmp(argv[1], "-c") == 0) {
            options.link_with_gcc = false;
            if (argc > 2) {
                options.src_filename = argv[2];
                options.output_filename = isolate_file_from_path(argv[2]);
                options.output_filename[strlen(options.output_filename) - 2] = '\0';
                return options;
            }
            else {
                printf("Error: Please specify a source file\n");
                exit(1);
            }
        }
        else {
            options.src_filename = argv[1];
            if (argc == 2) {
                options.output_filename = str_copy("a.out");
                return options;
            }
        }
    }
    else {
        printf("Error: Please specify a source file\n");
        exit(1);
    }
    if (argc > 2) { // Output executable name specified
        if (strcmp(argv[2], "-o") == 0) {
            if (argc > 3) {
                options.output_filename = str_copy(argv[3]);
            }
            else {
                printf("Error: Please specify an output executable filename\n");
                exit(1);
            }
        }
        else {
            options.output_filename = str_copy(argv[2]);
        }
    }
    return options;
}