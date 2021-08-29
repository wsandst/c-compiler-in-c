// Step 1: Preprocessing
// Convert a C source file into a list of Tokens
// Include additional files from preproccessor statements

// Support:
//      #include
//      #pragma once
//      #define (simple replace macros, not functions)
//      #undef
//      #ifdef
//      #ifndef

#pragma once
#include <string.h>
#include <stdio.h>

#include "util/file_helpers.h"
#include "util/vector.h"
#include "tokens.h"

// Should probably be a hashmap
struct PreprocessorTable {
    Vec* elems;
    int token_index;
    int current_file_index;
    char* current_file_dir;
    char* current_file;
};

typedef struct PreprocessorTable PreprocessorTable;

enum PreprocessorItemType {
    PP_DEFINE,
    PP_INCLUDED_FILE,
};

typedef enum PreprocessorItemType PreprocessorItemType;

struct PreprocessorItem {
    PreprocessorItemType type;
    char* name;
    char* value;
    bool include_file_only_once;
    Tokens define_value_tokens;
    bool ignore;
};

typedef struct PreprocessorItem PreprocessorItem;

// Turn the first file into a list of tokens
Tokens preprocess_first(char* filename, PreprocessorTable* table);

// Turn a file into a list of tokens
Tokens preprocess(char* filename, PreprocessorTable* table, bool is_stl_file);

Tokens preprocess_tokens(Tokens* tokens, PreprocessorTable* table);
void preprocess_token(Tokens* tokens, PreprocessorTable* table);

// Preprocess #include directive
void preprocess_include(Tokens* tokens, PreprocessorTable* table);

// Preprocess #define directive (replace macro)
void preprocess_define(Tokens* tokens, PreprocessorTable* table);

// Preprocess #undef directive (undefine)
void preprocess_undef(Tokens* tokens, PreprocessorTable* table);

// Preprocess identifiers and check if they match a define directive (replace macro)
void preprocess_ident(Tokens* tokens, PreprocessorTable* table);

// Preprocess #ifdef directives
void preprocess_ifdef(Tokens* tokens, PreprocessorTable* table);

// Preprocess #ifndef directives
void preprocess_ifndef(Tokens* tokens, PreprocessorTable* table);

// Scan for #endif directive, return offset from given token
int preprocess_scan_for_endif(Token* start_token, PreprocessorTable* table);

// =============== Preprocessor Table ===================
// Create a new PreprocessorTable
PreprocessorTable preprocessor_table_new();

// Free the memory of the PreprocessorTable
void preprocessor_table_free(PreprocessorTable* table);

// Lookup an element by name in the PreprocessorTable. Returns NULL if not found
PreprocessorItem* preprocessor_table_lookup(PreprocessorTable* table, char* name);

// Update the current directory which the file being preprocessed is in
void preprocessor_table_update_current_dir(PreprocessorTable* table, char* filepath);

// Get the current file directory
PreprocessorItem* preprocessor_table_get_current_file(PreprocessorTable* table);

// Insert an element into the PreprocessorTable
void preprocessor_table_insert(PreprocessorTable* table, PreprocessorItem item);

// Remove a potential element from the PreprocessorTable
int preprocessor_table_remove(PreprocessorTable* table, char* name);

// Add a simple define. Used for compiler specific defines etc
void preprocessor_table_add_simple_define(PreprocessorTable* table, char* name);

void preprocess_error(char* error_message, PreprocessorTable* table);