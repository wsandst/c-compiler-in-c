#include "tokens.h"

Tokens tokenize(char* src) {
    int src_length = strlen(src);
    Tokens tokens = tokens_new(src_length);

    // Start identifying tokens
    StrVector lines = str_split(src, '\n');
    for (size_t i = 0; i < lines.size; i++)
    {
        char* new_str = str_strip(lines.elems[i]);
        free(lines.elems[i]);
        lines.elems[i] = new_str;
    }

    str_vec_print(&lines);
    

    tokenize_preprocessor(&tokens, &lines);

    str_vec_print(&lines);

    tokens_print(&tokens);

    tokens_trim(&tokens);
    return tokens;
}

// Create a new Tokens object
Tokens tokens_new(int size) {
    Tokens tokens;
    tokens.size = size;
    tokens.elems = malloc(size*sizeof(Token));
    return tokens;
}

void tokens_free(Tokens *tokens) {
    free(tokens->elems);
}

// Remove NULL elements from the Token Array
void tokens_trim(Tokens *tokens) {
    // Count non-none tokens
    int token_count = 0;
    for (size_t i = 0; i < tokens->size; i++) {
        if (tokens->elems[i].type != TK_NONE) {
            token_count++;
        }
    }
    Token* new_token_array = malloc(token_count*sizeof(Token));
    tokens->size = token_count;
    // Add the tokens
    int j = 0;
    for (size_t i = 0; i < tokens->size; i++) {
        Token token = tokens->elems[i];
        if (token.type != TK_NONE) {
            new_token_array[j] = token;
            j++;
        }
    }
    free(tokens->elems);
    tokens->elems = new_token_array;
}

void tokenize_preprocessor(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    for (size_t i = 0; i < str_split->size; i++)
    {
        char* str = str_split->elems[i];
        // Is this a preprocessor line?
        if (str_startswith(str, "#")) {
            tokens->elems[src_pos].type = TK_PREPROCESSOR;
            tokens->elems[src_pos].data.string = str_copy(str);
            str_fill(str, strlen(str), ' ');
        }
        src_pos += strlen(str);
    }
}

void tokens_print(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token t = tokens->elems[i];
        if (t.type == TK_NONE) {
            continue;
        }
        printf("[T:%i, TS:%i ", t.type, t.sub_type);
        if (t.type == TK_COMMENT || t.type == TK_PREPROCESSOR) {
            printf("V: %s", t.data.string);
        } 
        else if (t.type == TK_VALUE) {
            printf("V: %i", t.data.ivalue);
        }
        printf("], ");
    }
    
}