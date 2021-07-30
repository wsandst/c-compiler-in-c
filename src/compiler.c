#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"

/*
TODO:
    Expressions:
        Proper left-to-right evaluation, it doesn't work like that currently
        Operator precedence (using Shunting Yard)
        Constant expressions (compile time expressions) (only really required for advanced switch cases)
        AND and OR short circuiting

    Functions, proper calling convention:
        Stack arguments, floating point arguments, floating point returns

    Globals:
        Issue: Globals become expressions. I can't really differentiate these from normal expressions
            Also, all the globals need to go at the top. How do I accomplish this?
            I create a separate data vec which only contains the data
            Then, I make the global an actual variable definition, then use this to add to the
            data vec. Join them in the end.

    Optimize scratch space usage

    Types:
        Type conversion:
            bool -> char -> short int -> int -> 
            unsigned int -> long -> unsigned -> 
            long long -> float -> double -> long double

    section .data
        var: dq 1
        .align 8


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
    SymbolTable* symbols = symbol_table_new();
    AST ast = parse(&tokens, symbols); 

    // Step 3: ASM Code Generation
    char* asm_src = generate_assembly(&ast);

    compile_asm(asm_src);

    // Free memory
    symbol_table_free(symbols);
    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(src);
    
    printf("Compilation complete\n");

    return 0;
}