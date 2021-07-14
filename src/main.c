#include <stdio.h>
#include "tokens.h"
#include "string_helpers.h"

int main() {
    char* str = "hello there";

    Tokens tokens = tokens_new(10);
    tokens.elems[2].type = TK_OP;
    tokens.elems[5].type = TK_OP;
    tokens_trim(&tokens);

    printf("Tokens: %i\n", tokens.size);
    
    return 0;
}