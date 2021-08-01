// Step 1: Preprocessing
// Convert a C source file into a list of Tokens
// Include additional files from preproccessor statements

// Planned support:
//      #include
//      #pragma
//      #define
//      #ifdef
//      #ifndef 

#pragma once
#include <string.h>
#include <stdio.h>

#include "util/file_helpers.h"
#include "util/vector.h"
#include "tokens.h"

typedef struct PreprocessorTable PreprocessorTable;
typedef enum PreprocessorItemType PreprocessorItemType;
typedef struct PreprocessorItem PreprocessorItem;

// Should probably be a hashmap
struct PreprocessorTable {
    Vec elems;
};

enum PreprocessorItemType {
    PP_DEFINE,
    PP_INCLUDED_FILE,
};

struct PreprocessorItem {
    PreprocessorItemType type;
    char* name;
    char* value;
};

// Turn the first file into a list of tokens
Tokens preprocess_first(char* filename);

// Turn a file into a list of tokens
Tokens preprocess(char* filename, PreprocessorTable* table);

Tokens preprocess_directives(Tokens* tokens, PreprocessorTable* table);

void preprocess_token(Tokens* tokens, PreprocessorTable* table, int token_index);
void preprocess_include(Tokens* tokens, PreprocessorTable* table, int token_index);

// =============== Preprocessor Table ===================
// Create a new PreprocessorTable
PreprocessorTable preprocessor_table_new();

// Free the memory of the PreprocessorTable
void preprocessor_table_free(PreprocessorTable* table);

// Lookup an element by name in the PreprocessorTable. Returns NULL if not found
PreprocessorItem* preprocessor_table_lookup(PreprocessorTable* table, char* name);

// Insert an element into the PreprocessorTable
void preprocessor_table_insert(PreprocessorTable* table, PreprocessorItem item);

void preprocess_error(char* error_message);