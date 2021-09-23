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
    if (ASSEMBLE_DEBUG) {
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
    // cases:
    //    ccic file.c (compile to a.out)
    //    ccic file.c -o executable (compile to executable)
    //    ccic -c file.c (compile to object file, name same as file)
    //    ccic -c file.c -o object_file (compile to object file of specified name)
    CompileOptions options;
    options.link_with_gcc = true;
    if (argc > 1) {
        if (strcmp(argv[1], "-c") == 0 && argc > 2) { // Compile to object file
            options.link_with_gcc = false;
            options.src_filename = argv[2];
            if (argc == 3) {
                options.output_filename = isolate_file_from_path(argv[2]);
                options.output_filename[strlen(options.output_filename) - 2] = '\0';
                return options;
            }
            else if (argc > 4 && strcmp(argv[3], "-o") == 0) {
                options.output_filename = str_copy(argv[4]);
            }
            else {
                fprintf(stderr, "Error: Please specify a source file\n");
                exit(1);
            }
        } // Compile to executable
        else {
            options.src_filename = argv[1];
            if (argc == 2) {
                options.output_filename = str_copy("a.out");
                return options;
            }
            else if (argc > 3 && strcmp(argv[2], "-o") == 0) {
                options.output_filename = str_copy(argv[3]);
            }
            else {
                fprintf(stderr, "Error: Please specify a source file\n");
                exit(1);
            }
        }
    }
    else {
        fprintf(stderr, "Error: Please specify a source file\n");
        exit(1);
    }
    return options;
}