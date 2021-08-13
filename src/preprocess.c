#include "preprocess.h"

Tokens preprocess_first(char* filename) {
    PreprocessorTable table = preprocessor_table_new();
    Tokens tokens = preprocess(filename, &table);
    tokens_trim(&tokens);
    preprocessor_table_free(&table);
    return tokens;
}

Tokens preprocess(char* filename, PreprocessorTable* table) {
    char* filename_with_dir = str_add(table->current_file_dir, filename);
    char* src = load_file_to_string(filename_with_dir);
    preprocessor_table_update_current_dir(table, filename);

    Tokens tokens = tokenize(src);
    
    preprocess_tokens(&tokens, table);

    free(src);
    free(table->current_file_dir);
    free(filename_with_dir);
    return tokens;
}

Tokens preprocess_tokens(Tokens* tokens, PreprocessorTable* table) {
    for (size_t i = 0; i < tokens->size; i++) {
        table->token_index = i;
        preprocess_token(tokens, table);
        i = table->token_index;
    }
    return *tokens;
}

void preprocess_token(Tokens* tokens, PreprocessorTable* table) {
    Token* token = tokens_get(tokens, table->token_index);
    if (token->type == TK_PREPROCESSOR) {
        if (str_startswith(token->string_repr, "#include")) {
            preprocess_include(tokens, table);
        }
        else if (str_startswith(token->string_repr, "#define")) {
            preprocess_define(tokens, table);
        }
        else if (str_startswith(token->string_repr, "#pragma once")) {
            PreprocessorItem* cur_file = preprocessor_table_get_current_file(table);
            cur_file->include_file_only_once = true;
        }
        else {
            preprocess_error("Unknown preprocess directive encountered");
        }
    }
    else if (token->type == TK_IDENT) {
        preprocess_ident(tokens, table);
    }
}

void preprocess_include(Tokens* tokens, PreprocessorTable* table) {
    Token* token = tokens_get(tokens, table->token_index);
    // Isolate the include filename
    StrVector str_vec = str_split(token->string_repr, ' ');
    char* file_str = str_vec.elems[1];
    if (file_str[0] != '\"') { // We do not support STL yet
        return;
    }
    file_str[0] = ' ';
    file_str[strlen(file_str) - 1] = ' ';
    file_str = str_strip(file_str);

    // If file already is in table and has pragma once, skip
    PreprocessorItem* item = preprocessor_table_lookup(table, file_str);
    if (item && item->include_file_only_once) {
        str_vec_free(&str_vec);
        free(file_str);
        return;
    }
    // Add it to the preprocessor table
    PreprocessorItem file_item;
    file_item.type = PP_INCLUDED_FILE;
    file_item.name = file_str;

    preprocessor_table_insert(table, file_item);

    // Consumed this token, set it to none
    free(token->string_repr);
    token->type = TK_NONE;
    token->requires_string_free = false;
    
    // Send the include file into the preprocessor
    PreprocessorTable next_table = *table;
    next_table.current_file_index = next_table.elems->size-1;

    Tokens file_tokens = preprocess(file_str, &next_table);
    tokens_trim(&file_tokens);
    // Remove the EOF token
    Token* last_token = vec_peek(&file_tokens.elems);
    last_token->type = TK_NONE;
    // Insert the include file tokens at the preprocessor token location
    tokens = tokens_insert(tokens, &file_tokens, table->token_index);
    // Free used memory
    str_vec_free(&str_vec);
    vec_free(&file_tokens.elems);
    table->token_index += file_tokens.size;
}

void preprocess_define(Tokens* tokens, PreprocessorTable* table) {
    Token* token = tokens_get(tokens, table->token_index);
    // Isolate everything after define
    StrVector str_vec = str_split(token->string_repr, ' ');
    char* define_ident = str_copy(str_vec.elems[1]);
    StrVector str_vec_value = str_vec_slice(&str_vec, 2, str_vec.size);
    char* define_value = str_vec_join_with_delim(&str_vec_value, ' '); 

    Tokens define_value_tokens = tokenize(define_value);
    // Remove the EOF token
    Token* last_token = vec_peek(&define_value_tokens.elems);
    last_token->type = TK_NONE;
    tokens_trim(&define_value_tokens);
    preprocess_tokens(&define_value_tokens, table);
    // Consumed this token, set it to none
    free(token->string_repr);
    token->type = TK_NONE;
    token->requires_string_free = false;

    // If file already is in table, override
    PreprocessorItem* item = preprocessor_table_lookup(table, define_value);
    if (item) {
        // Just overwrite with the new value tokens
        vec_free(&item->define_value_tokens.elems);
        item->define_value_tokens = define_value_tokens;
        return;
    }
    // Add it to the preprocessor table
    PreprocessorItem define_item;
    define_item.type = PP_DEFINE;
    define_item.name = define_ident;
    define_item.define_value_tokens = define_value_tokens;
    preprocessor_table_insert(table, define_item);

    str_vec_free(&str_vec);
    free(define_value);
}

void preprocess_ident(Tokens* tokens, PreprocessorTable* table) {
    // Replace identifiers which are defines with the define tokens
    Token* token = tokens_get(tokens, table->token_index);
    PreprocessorItem* item = preprocessor_table_lookup(table, token->string_repr);
    if (item == NULL) {
        return;
    }
    // This is a define identifier, replace with define tokens

    // Consumed this token, set it to none
    free(token->string_repr);
    token->type = TK_NONE;
    token->requires_string_free = false;
    Tokens insert_tokens = tokens_copy(&item->define_value_tokens);
    tokens = tokens_insert(tokens, &insert_tokens, table->token_index);
    free(insert_tokens.elems.elems);
    table->token_index += item->define_value_tokens.size;
}

// ================ Preprocessor Table ===================

PreprocessorTable preprocessor_table_new() {
    PreprocessorTable table;
    table.elems = vec_new_dyn(sizeof(PreprocessorItem));
    table.token_index = 0;
    table.current_file_dir = NULL;
    table.current_file_index = 0;
    return table;
}

void preprocessor_table_free(PreprocessorTable* table) {
    for (size_t i = 0; i < table->elems->size; i++)
    {
        PreprocessorItem* item = vec_get(table->elems, i);
        if (item->type == PP_INCLUDED_FILE && item->name != NULL) {
            free(item->name);
        }
        else if (item->type == PP_DEFINE && item->name != NULL) {
            tokens_free(&item->define_value_tokens);
            free(item->name);
        }
    }
    vec_free(table->elems);
    free(table->elems);
}

void preprocessor_table_update_current_dir(PreprocessorTable* table, char* filepath) {
    if (table->current_file_dir == NULL) {
        table->current_file_dir = "";
    }
    char* file_dir = isolate_file_dir(filepath);
    table->current_file_dir = str_add(table->current_file_dir, file_dir);
    free(file_dir);
}

PreprocessorItem* preprocessor_table_lookup(PreprocessorTable* table, char* name) {
    for (size_t i = 0; i < table->elems->size; i++)
    {
        PreprocessorItem* item = vec_get(table->elems, i);
        if (strcmp(item->name, name) == 0) {
            return item;
        }
    }
    return NULL;
}

PreprocessorItem* preprocessor_table_get_current_file(PreprocessorTable* table) {
    return vec_get(table->elems, table->current_file_index);
}

void preprocessor_table_insert(PreprocessorTable* table, PreprocessorItem item) {
    vec_push(table->elems, &item);
}

void preprocess_error(char* error_message) {
    fprintf(stderr, "Preprocess error: %s\n", error_message);
    exit(1); 
}
