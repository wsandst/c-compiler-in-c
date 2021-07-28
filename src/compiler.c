#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"

/*
TODO:
    Expressions:
        Proper left-to-right evaluation, it doesn't work like that currently
        Operator precedence (using Shunting Yard)
        I might be using too much scratch memory, not sure if what I'm doing
        currently is optimal

    Gotos

    Function parameters, proper calling convention:
        Return 0 from functions if there is no return statement, won't follow spec otherwise

    Loops:
        Break, conntinue
        Refactor into a single loop construct instead?
        At least while and for should technically be able to be the same construct
        just have to insert the for stuff at the start and the end. How to insert at the end though?
        do while needs to be separate. I could iterate over the statements until I find the end, then insert it
        

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