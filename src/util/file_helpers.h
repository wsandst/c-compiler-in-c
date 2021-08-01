#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "string_helpers.h"

// Load a file to a C string
char* load_file_to_string(char* filename);
// Write a C string to file
void write_string_to_file(char* filename, char *src);
// Compile Intel-syntax ASM using the NASM assembler and link with gcc
void compile_asm(char *asm_src);
// Isolate the directory of a filepath
char* isolate_file_dir(char* filepath);