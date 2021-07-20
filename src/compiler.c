#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"


int main() {
    char* src = load_file_to_string("../test/resources/test_source.c");

    // Step 1: Tokenization
    Tokens tokens = tokenize(src);
    //tokens_print(&tokens);

    // Step 2: AST Parsing
    AST ast = parse(&tokens); 

    // Step 3: ASM Code Generation
    //char* asm_src = load_file_to_string("../test/resources/hello_world.asm");
    char* asm_src = generate_assembly(&ast);

    compile_asm(asm_src);

    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(src);
    
    return 0;
}