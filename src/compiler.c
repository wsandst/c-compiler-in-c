#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"

/*
TODO:
    Expressions:
        Implement parentheses
        Proper left-to-right evaluation, it doesn't work like that currently
        Operator precedence (using Shunting Yard)

    If:
        Else statements, handle else ifs too
    Gotos:

    While loops
    For loops
    Function parameters, proper calling convention
*/

int main(int argc, char *argv[]) {
    char* src_path;
    if (argc > 1) {
        src_path = argv[1];
    }
    else {
        printf("Error: Please specify a source file\n");
        exit(1);
    }

    printf("Compiling source file \"%s\"\n", src_path);

    char* src = load_file_to_string(src_path);

    // Step 1: Tokenization
    Tokens tokens = tokenize(src);
    //tokens_print(&tokens);

    // Step 2: AST Parsing
    AST ast = parse(&tokens); 

    // Step 3: ASM Code Generation
    char* asm_src = generate_assembly(&ast);

    compile_asm(asm_src);

    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(src);
    
    printf("Compilation complete\n");

    return 0;
}