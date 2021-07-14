#include "tokens.h"

Tokens tokenize(char* source) {

}


// Create a new Tokens object
Tokens tokens_new(int initial_size) {
    Tokens tokens;
    tokens.size = 0;
    tokens.max_size = initial_size;
    tokens.token_array = malloc(initial_size*sizeof(Token));
    return tokens;
}

void tokens_free(Tokens *tokens) {
    free(tokens->token_array);
}

// Add a token object to the end of the token vector
void tokens_push(Tokens *tokens, Token token) {
    tokens->size++;
    if (tokens->size >= tokens->max_size) {
        tokens_realloc(tokens, tokens->max_size*2);
    }
    tokens->token_array[tokens->size] = token;
}

void tokens_realloc(Tokens *tokens, int new_size) {
    tokens->token_array = realloc(tokens->token_array, new_size*sizeof(Token));
    tokens->max_size = new_size;
}

void tokens_sort(Tokens *tokens) {

}

// 
