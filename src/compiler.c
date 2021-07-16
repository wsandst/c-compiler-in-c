#include "tokens.h"
#include "parser.h"
#include "file_helpers.h"

int main() {
    char* src = load_file_to_string("test/resources/tokenizer_test.c");

    Tokens tokens = tokenize(src);
    //AST ast = parse(&tokens);
    //tokens_print(&tokens);

    tokens_free(&tokens);
    free(src);
    
    return 0;
}