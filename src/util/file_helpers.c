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

// Parse commandline options using getopt
// switch to getopt_long to allow more arguments?
// Future arguments to support: --keepasm, -g (debug info as comments in the assembly)
CompileOptions parse_compiler_options(int argc, char** argv) {
    opterr = 0;
    CompileOptions options;
    options.link_with_gcc = true;
    options.output_filename = "a.out";
    bool output_file_set = false;
    int arg_c;
    while ((arg_c = getopt(argc, argv, "co:")) != -1) {
        switch (arg_c) {
            case 'o':
                options.output_filename = optarg;
                output_file_set = true;
                break;
            case 'c':
                options.link_with_gcc = false;
                break;
            case '?':
                if (optopt == 'o') {
                    fprintf(stderr, "Option '-%c' requires a file argument\n", optopt);
                }
                else {
                    fprintf(stderr, "Unknown option '-%c' provided\n", optopt);
                }
                exit(EXIT_FAILURE);
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        options.src_filename = argv[optind];
        if (!output_file_set && !options.link_with_gcc) {
            // If an output filename was not given and we are linking, use
            // the source file as output name
            options.output_filename = isolate_file_from_path(options.src_filename);
            options.output_filename[strlen(options.output_filename) - 1] = 'o';
        }
        else {
            options.output_filename = str_copy(options.output_filename);
        }
    }
    return options;
}