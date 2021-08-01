#include "preprocess.h"


char* first_file_dir;

Tokens preprocess_first(char* filename) {
    PreprocessorTable table = preprocessor_table_new();
    first_file_dir = isolate_file_dir(filename);
    Tokens tokens = preprocess(filename, &table);
    tokens_trim(&tokens);
    preprocessor_table_free(&table);
    free(first_file_dir);
    return tokens;
}

Tokens preprocess(char* filename, PreprocessorTable* table) {
    char* src = load_file_to_string(filename);
    Tokens tokens = tokenize(src);
    preprocess_directives(&tokens, table);
    free(src);
    return tokens;
}

Tokens preprocess_directives(Tokens* tokens, PreprocessorTable* table) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token* token = &tokens->elems[i];
        if (token->type == TK_PREPROCESSOR) {
            preprocess_token(tokens, table, i);
        }
    }
    return *tokens;
}

void preprocess_token(Tokens* tokens, PreprocessorTable* table, int token_index) {
    Token* token = &tokens->elems[token_index];
    if (str_startswith(token->string_repr, "#include")) {
        preprocess_include(tokens, table, token_index);
    }
    else {
        preprocess_error("Unknown preprocess directive encountered");
    }
}

void preprocess_include(Tokens* tokens, PreprocessorTable* table, int token_index) {
    Token* token = &tokens->elems[token_index];
    // Isolate the include file
    StrVector str_vec = str_split(token->string_repr, ' ');
    char* file_str = str_vec.elems[1];
    file_str[0] = ' ';
    file_str[strlen(file_str) - 1] = ' ';
    file_str = str_strip(file_str);
    PreprocessorItem file_item;
    file_item.type = PP_INCLUDED_FILE;
    file_item.name = file_str;
    preprocessor_table_insert(table, file_item);

    free(token->string_repr);
    token->type = TK_NONE;
    
    char* file_with_dir_str = str_add(first_file_dir, file_str);
    Tokens file_tokens = preprocess(file_with_dir_str, table);
    tokens_trim(&file_tokens);
    // Remove the EOF token
    file_tokens.elems[file_tokens.size-1].type = TK_NONE;
    tokens = tokens_insert(tokens, &file_tokens, token_index);
    //tokens_print(tokens);
    str_vec_free(&str_vec);
    free(file_tokens.elems);
    free(file_with_dir_str);
}

// ================ Preprocessor Table ===================

PreprocessorTable preprocessor_table_new() {
    PreprocessorTable table;
    table.elems = vec_new(sizeof(PreprocessorItem));
    return table;
}

void preprocessor_table_free(PreprocessorTable* table) {
    for (size_t i = 0; i < table->elems.size; i++)
    {
        PreprocessorItem* item = vec_get(&table->elems, i);
        if (item->type == PP_INCLUDED_FILE && item->name != NULL) {
            free(item->name);
        }
    }
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

void preprocess_error(char* error_message) {
    fprintf(stderr, "Preprocess error: %s\n", error_message);
    exit(1); 
}
