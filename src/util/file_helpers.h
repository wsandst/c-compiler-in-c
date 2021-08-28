#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "string_helpers.h"

struct CompileOptions {
    char* src_filename;
    char* output_filename;
    bool link_with_gcc;
};

typedef struct CompileOptions CompileOptions;

// Load a file to a C string
char* load_file_to_string(char* filename);

// Write a C string to file
void write_string_to_file(char* filename, char* src);

// Compile Intel-syntax ASM using the NASM assembler and link with gcc
void compile_asm(char* asm_src, CompileOptions compile_options);

// Isolate the directory of a filepath (exclude the file)
char* isolate_file_dir(char* filepath);
// Isolate the file of a filepath (exclude the dirs)
char* isolate_file_from_path(char* filepath);

// Parse compiler command line options
CompileOptions parse_compiler_options(int argc, char** argv);