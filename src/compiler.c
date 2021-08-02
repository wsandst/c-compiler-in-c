#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Next time:
        Preprocessor: Pragma once
        Start investigating type system

    Expressions:
        Constant expressions (compile time expressions) (only really required for advanced switch cases):
            I can do this as a post-processing step on the parse tree, or perhaps in the binary/unary
            op handling.

    Functions, proper calling convention:
        Stack arguments, floating point arguments, floating point returns

    Types:
        Type conversion:
            bool -> char -> short int -> int -> 
            unsigned int -> long -> unsigned -> 
            long long -> float -> double -> long double
        Arrays

    Preprocessor:
        I should refactor so that I process the files recursively, 
        not iteratively like I do currently. This way I can start handling pragma once
        and such correctly
        Pragma once:
            I'm keeping track of the filenames, just need to mark with pragma once and
            only include if not seen before
        Defines
        Includes:
            STD headers. I should take from GCC and remove all the macros
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

    //char* src = load_file_to_string(src_path);

    // Step 1: Preprocessing + Tokenization
    Tokens tokens = preprocess_first(src_path);
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
    
    printf("Compilation complete\n");

    return 0;
}