#include "tokens.h"
#include "parser.h"
#include "file_helpers.h"

int main() {
    char* src = load_file_to_string("test/resources/test_source.c");

    Tokens tokens = tokenize(src);
    tokens_print(&tokens);
    AST ast = parse(&tokens);


    tokens_free(&tokens);
    ast_free(&ast);
    free(src);
    
    return 0;
}