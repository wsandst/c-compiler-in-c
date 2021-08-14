#include "preprocess.h"

Tokens preprocess_first(char* filename) {
    PreprocessorTable table = preprocessor_table_new();
    Tokens tokens = preprocess(filename, &table, false);
    tokens_trim(&tokens);
    preprocessor_table_free(&table);
    return tokens;
}

Tokens preprocess(char* filename, PreprocessorTable* table, bool is_stl_file) {
    char* filename_with_dir;
    if (!is_stl_file) { // Normal file
        filename_with_dir = str_add(table->current_file_dir, filename);
    }
    else { // STL file, stored under libc/
        filename_with_dir = str_add("libc/", filename);
    }
    table->current_file = filename_with_dir;
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
        else if (str_startswith(token->string_repr, "#ifdef")) {
            preprocess_ifdef(tokens, table);
        }
        else if (str_startswith(token->string_repr, "#ifndef")) {
            preprocess_ifndef(tokens, table);
        }
        else if (str_startswith(token->string_repr, "#undef")) {
            preprocess_undef(tokens, table);
        }
        else {
            preprocess_error("Unknown preprocess directive encountered", table);
        }
    }
    else if (token->type == TK_IDENT) {
        preprocess_ident(tokens, table);
    }
}

void preprocess_include(Tokens* tokens, PreprocessorTable* table) {
    Token* token = tokens_get(tokens, table->token_index);
    // Isolate the include filename
    StrVector str_vec = str_split_on_whitespace(token->string_repr);
    char* file_str = str_vec.elems[1];
    bool is_stl_file = false;
    if (file_str[0] == '\"') { // We do not support STL yet
        is_stl_file = false;
    }
    else if (file_str[0] == '<') {
        is_stl_file = true;
    }
    else {
        preprocess_error("Incorrectly formatted include encountered!", table);
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
    file_item.include_file_only_once = false;

    preprocessor_table_insert(table, file_item);

    // Consumed this token, set it to none
    token->type = TK_NONE;
    
    // Send the include file into the preprocessor
    PreprocessorTable next_table = *table;
    next_table.current_file_index = next_table.elems->size-1;

    Tokens file_tokens = preprocess(file_str, &next_table, is_stl_file);
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
    StrVector str_vec = str_split_on_whitespace(token->string_repr);
    char* define_ident = str_copy(str_vec.elems[1]);
    StrVector str_vec_value = str_vec_slice(&str_vec, 2, str_vec.size);
    char* define_value = str_vec_join_with_delim(&str_vec_value, ' '); 

    Tokens define_value_tokens;
    if (str_vec_value.size > 0) { // Define has a replace value we should use
        define_value_tokens = tokenize(define_value);
    }
    else { // This is an empty define, if used should produce no token
        define_value_tokens = tokens_new(1);
    }

    // Remove the EOF token
    Token* last_token = vec_peek(&define_value_tokens.elems);
    last_token->type = TK_NONE;
    tokens_trim(&define_value_tokens);
    preprocess_tokens(&define_value_tokens, table);

    // Consume this token, set it to none
    token->type = TK_NONE;

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

// Preprocess #undef directive (undefine)
void preprocess_undef(Tokens* tokens, PreprocessorTable* table) {
    // Remove item from preprocessor table
    Token* token = tokens_get(tokens, table->token_index);
    // Isolate everything after define
    StrVector str_vec = str_split_on_whitespace(token->string_repr);
    char* define_ident = str_copy(str_vec.elems[1]);

    preprocessor_table_remove(table, define_ident);

    // Consume this token, set it to none
    token->type = TK_NONE;

    str_vec_free(&str_vec);
    free(define_ident);
}

void preprocess_ident(Tokens* tokens, PreprocessorTable* table) {
    // Replace identifiers which are defines with the define tokens
    Token* token = tokens_get(tokens, table->token_index);
    PreprocessorItem* item = preprocessor_table_lookup(table, token->string_repr);
    if (item == NULL) {
        return;
    }
    // This is a define identifier, replace with define tokens

    // Consume this token, set it to none
    token->type = TK_NONE;
    Tokens insert_tokens = tokens_copy(&item->define_value_tokens);
    tokens = tokens_insert(tokens, &insert_tokens, table->token_index);
    free(insert_tokens.elems.elems);
    table->token_index += item->define_value_tokens.size;
}

// Preprocess #ifdef directives
void preprocess_ifdef(Tokens* tokens, PreprocessorTable* table) {
    Token* token = tokens_get(tokens, table->token_index);
    // Isolate #ifdef identifier
    StrVector str_vec = str_split_on_whitespace(token->string_repr);
    if (str_vec.size < 2) {
        preprocess_error("#ifdef directive has no identifier!", table);
    }
    char* define_ident = str_copy(str_vec.elems[1]);
    int endif_offset = preprocess_scan_for_endif(token, table);
    if (preprocessor_table_lookup(table, define_ident)) { // Value defined, use code
        // Consume this token, set it to none
        token->type = TK_NONE;

        // Consume matching #endif
        token += endif_offset;
        token->type = TK_NONE;
    }
    else { // Value not defined, cut out the tokens by setting them to none
        Token* end_token = token + endif_offset;
        while (token <= end_token) {
            token->type = TK_NONE;
            token++;
        }
    }

    str_vec_free(&str_vec);
    free(define_ident);
}

// Preprocess #ifndef directives
void preprocess_ifndef(Tokens* tokens, PreprocessorTable* table) {
    Token* token = tokens_get(tokens, table->token_index);
    // Isolate #ifdef identifier
    StrVector str_vec = str_split_on_whitespace(token->string_repr);
    if (str_vec.size < 2) {
        preprocess_error("#ifndef directive has no identifier!", table);
    }
    char* define_ident = str_copy(str_vec.elems[1]);
    int endif_offset = preprocess_scan_for_endif(token, table);
    if (!preprocessor_table_lookup(table, define_ident)) { // Value not defined, use code
        // Consume this token, set it to none
        token->type = TK_NONE;

        // Consume matching #endif
        token += endif_offset;
        token->type = TK_NONE;
    }
    else { // Value defined, cut out the tokens by setting them to none
        Token* end_token = token + endif_offset;
        while (token <= end_token) {
            token->type = TK_NONE;
            token++;
        }
    }

    str_vec_free(&str_vec);
    free(define_ident);
}

// Scan for #endif directive, return offset from given token
int preprocess_scan_for_endif(Token* start_token, PreprocessorTable* table) {
    // Simple stack-based match search
    int offset = 0;
    int endifs_left = 0; // Used as stack
    while (start_token->type != TK_EOF) {
        if (start_token->type == TK_PREPROCESSOR) {
            if (str_startswith(start_token->string_repr, "#ifdef") ||
                    str_startswith(start_token->string_repr, "#ifndef")) {
                endifs_left++; // Push
            }
            else if (str_startswith(start_token->string_repr, "#endif")) {
                endifs_left--; // Pop
            }
        }
        if (endifs_left == 0) { // Stack empty, found match
            return offset;
        }
        start_token++;
        offset++;
    }
    preprocess_error("#ifdef/#ifndef directive has no matching #endif!", table);
    return -1;
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
        if (!item->ignore && strcmp(item->name, name) == 0) {
            return item;
        }
    }
    return NULL;
}

PreprocessorItem* preprocessor_table_get_current_file(PreprocessorTable* table) {
    return vec_get(table->elems, table->current_file_index);
}

void preprocessor_table_insert(PreprocessorTable* table, PreprocessorItem item) {
    item.ignore = false;
    vec_push(table->elems, &item);
}

int preprocessor_table_remove(PreprocessorTable* table, char* name) {
    PreprocessorItem* item = preprocessor_table_lookup(table, name);
    if (item == NULL) { // This item does not exist, do nothing
        return 0;
    }
    // Removing items is expensive, just ignore this item from now on
    item->ignore = true;
    return 1;
}

void preprocess_error(char* error_message, PreprocessorTable* table) {
    fprintf(stderr, "Preprocess error: %s (in file \"%s\")\n", error_message, table->current_file);
    exit(1); 
}
