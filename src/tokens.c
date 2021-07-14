#include "tokens.h"

Tokens tokenize(char* source) {

}

// Create a new Tokens object
Tokens tokens_new(int size) {
    Tokens tokens;
    tokens.size = size;
    tokens.token_array = malloc(size*sizeof(Token));
    return tokens;
}

void tokens_free(Tokens *tokens) {
    free(tokens->token_array);
}

// Remove NULL elements from the Token Array
void tokens_trim() {

}
