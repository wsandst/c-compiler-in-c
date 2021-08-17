#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Current:
        Improve function arg handling:
            Save on stack

        Structs, enums, unions


    Refactoring:
        Refactor parser
        Cleanup pointer lhs deref r12 pushing

    Expressions:
        Constant expressions (compile time expressions) (only really required for advanced switch cases):
            I can do this as a post-processing step on the parse tree, or perhaps in the binary/unary
            op handling.

    Functions, proper calling convention:
        The function argument handling needs to be improved:
            Keep track of function argument types in the function object, for call arg casting
            Improve handling of calling convention, currently can't handle certain edge cases
            This restructuring will be necessary for easier struct implemention

    Types:
        Floating point:
            Operators left: ++, --
        Arrays:
            Infer array size from initializer size
            Treat x[] as *x
            These are connected. int x[] is equivalent with int* x;
            sizeof array
            sizeof self. Might even work currently?
        Constants:
            Just ignore the keyword for now
    
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

    // Save ASM src to file and compile with NASM
    compile_asm(asm_src, "output");

    // Free memory
    symbol_table_free(symbols);
    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    
    printf("Compilation complete\n");

    return 0;
}