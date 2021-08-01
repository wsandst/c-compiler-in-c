#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"

/*
TODO:
    Tomorrow:
        AND and OR short circuiting
        Function declarations
        Simple pointers
        Preproccessor planning, architecture

    Expressions:
        Constant expressions (compile time expressions) (only really required for advanced switch cases)
        AND and OR short circuiting

    a && b && c. If a false,, skip to end of c.

    Functions, proper calling convention:
        Function declarations
        Stack arguments, floating point arguments, floating point returns

    Types:
        Type conversion:
            bool -> char -> short int -> int -> 
            unsigned int -> long -> unsigned -> 
            long long -> float -> double -> long double
        Arrays

    Preprocessor:
        Includes
        Defines
        Pragma once
    


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
    char* asm_src = generate_assembly(&ast, symbols);

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