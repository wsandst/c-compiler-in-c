#include "tokens.h"

Tokens tokenize(char* src) {
    int src_length = strlen(src);
    Tokens tokens = tokens_new(src_length);

    // Start identifying tokens

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
