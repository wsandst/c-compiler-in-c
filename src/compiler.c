#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Refactoring:
        Refactor parser:
            parse_program should run a subset of parse_statement, this should
            decrease code duplication (and allow for local functions in the future)
        Cleanup pointer lhs deref r12 pushing
        Refactor constant expression handling:
            Everything is very fragmented currently, handling of globals/cases/enums should be
            the same, not different as it is currently

    Functions, proper calling convention:
        The function argument handling needs to be improved:
            Improve handling of calling convention, currently can't handle certain edge cases
        Variadic function definitions, not just declarations/calling

    Structs:

    Types:
        Arrays:
            Infer array size from initializer size
            Treat x[] as *x
            These are connected. int x[] is equivalent with int* x;
            char* x = {"a", "b", "c"} should work if char x[] works, same construct
    
    Tokenizer:
        Fix nested strings within comments, comments within strings, escape characters etc

    Intentional deficits:
        Constants are not constant
        Only literals are allowed for constant expressions
        Variable length arrays are not implemented
        Comma operator and ternary operator is not implemented
        Certain float operators have been skipped
        No function pointers or local functions

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
    //tokens_pretty_print(&tokens);

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