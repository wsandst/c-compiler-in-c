#pragma once
#include <stdio.h>
#include <stdlib.h>

char* load_file_to_string(char* filename);
void write_string_to_file(char* filename, char *src);
void compile_asm(char *asm_src);