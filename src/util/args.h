#pragma once
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include "string_helpers.h"

struct CompileOptions {
    char* src_filename;
    char* output_filename;
    bool link_with_gcc;
    bool debug_annotate_assembly;
    bool keep_assembly;
};

typedef struct CompileOptions CompileOptions;

CompileOptions parse_compiler_options(int argc, char** argv);

// Isolate the directory of a filepath (exclude the file)
char* isolate_file_dir(char* filepath);
// Isolate the file of a filepath (exclude the dirs)
char* isolate_file_from_path(char* filepath);