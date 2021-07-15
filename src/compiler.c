#include "tokens.h"
#include "file_helpers.h"

int main() {
    char* src = load_file_to_string("test/resources/tokenizer_test.c");

    Tokens tokens = tokenize(src);
    tokens_print(&tokens);

    free(src);
    
    return 0;
}