#include "preprocess.h"

Tokens preprocess(char* filename, PreprocessorTable table) {
    char* src = load_file_to_string(filename);
    Tokens tokens = tokenize(src);
    free(src);
    return tokens;
}

Tokens preprocess_directives(Tokens* tokens, PreprocessorTable table) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        /* code */
    }
    return *tokens;
}

PreprocessorTable preprocessor_table_new() {
    PreprocessorTable table;
    table.elems = vec_new(sizeof(PreprocessorItem));
    return table;
}

void preprocessor_table_free(PreprocessorTable* table) {
    vec_free(&table->elems);
}

PreprocessorItem* preprocessor_table_lookup(PreprocessorTable* table, char* name) {
    for (size_t i = 0; i < table->elems.size; i++)
    {
        PreprocessorItem* item = vec_get(&table->elems, i);
        if (strcmp(item->name, name) == 0) {
            return item;
        }
    }
    return NULL;
}

void preprocessor_table_insert(PreprocessorTable* table, PreprocessorItem item) {
    vec_push(&table->elems, &item);
}

// I need a way of inserting tokens where another token was