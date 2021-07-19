#include "tokens.h"
#include "parser.h"
#include "file_helpers.h"


int main() {
    char* src = load_file_to_string("../test/resources/test_source.c");
    //char* asm_src = load_file_to_string("../test/resources/hello_world.asm");

    Tokens tokens = tokenize(src);
    tokens_print(&tokens);
    AST ast = parse(&tokens);

    //compile_asm(asm_src);

    tokens_free(&tokens);
    ast_free(&ast);
    free(src);
    
    return 0;
}