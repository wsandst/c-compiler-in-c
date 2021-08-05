#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Next time:
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
        sizeof:
            long double not implemented
        Casting: Implicit vs explicit casting
        Explicit casting:
        Implicit casting:
            The binary operator keeps track of the largest type.
            Before we perform the operation we then perform promotion of the lower type up to it
        
        Promotion: integers, essentially do nothing
        Promote to float: Convert integer to float
        Demote to int: Convert float to int

        Once a binary operation is defined (both lhs and rhs done),
        I need to label the operation with whatever type is wider.
        The casting unary operator also changes this

        Operations:
            Floating point
            Pointer (+, -, ++, --)

    Preprocessor:
        Defines:
            Simple defines, ifndef, ifdef. Goal is to support simple include headers
            and simple constant defines.
        Includes:
            Support STD headers. I should take from GCC and remove all the macros
    
    Tokenizer:
        Fix nested strings within comments, comments within strings, escape characters etc
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