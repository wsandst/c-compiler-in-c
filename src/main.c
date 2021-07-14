#include <stdio.h>
#include "tokens.h"

int main() {
    Tokens tokens = tokens_new(1);
    Token test_token;
    printf("size:%i, max_size:%i\n", tokens.size, tokens.max_size);
    for (int i = 0; i < 10; i++)
    {
        tokens_push(&tokens, test_token);
        printf("size:%i, max_size:%i\n", tokens.size, tokens.max_size);
    }
    
    return 0;
}