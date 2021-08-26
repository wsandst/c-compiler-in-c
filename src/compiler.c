#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Refactoring:
        Refactor parser:
            parse_program should run a subset of parse_statement, this should
            decrease code duplication (and allow for local functions in the future)
        Refactor constant expression handling:
            Everything is very fragmented currently, handling of globals/cases/enums should be
            the same, not different as it is currently
        Refactor codegen, decrease unnecessary assembly:
            Leaf nodes vs non-leaf nodes
            It is unneccessary to push on leaf nodes.

    Functions:
        Variadic function definitions, not just declarations/calling:
            https://blog.nelhage.com/2010/10/amd64-and-va_arg/
            Just recreate that struct with va_start, push the registers to stack, pass it by pointer  

    Types:
        Arrays:
            Infer array size from initializer size
            Treat x[] as *x
            These are connected. int x[] is equivalent with int* x;
            char* x = {"a", "b", "c"} should work if char x[] works, same construct
            Variable length arrays: just add to rsp, then set address to this
    
    Try compiling one of the test programs

    Intentional deficits:
        Constants variables are not evaluated at compile-time
        Only literals are allowed for most constant expressions
        Variable length arrays are not implemented
        Comma operator and ternary operator is not implemented
        Certain float operators have been skipped
        No function pointers or local functions
        Floats are 64 bit, not 32 bit. Only really messes with scanf
        All integers are signed no matter what
        No unions

*/

int main(int argc, char** argv) {
    CompileOptions compile_options = parse_compiler_options(argc, argv);

    printf("Compiling source file \"%s\"\n", compile_options.src_filename);

    // Step 1: Preprocessing + Tokenization
    PreprocessorTable table = preprocessor_table_new();
    Tokens tokens = preprocess_first(compile_options.src_filename, &table);
    //tokens_pretty_print(&tokens);

    // Step 2: AST Parsing
    SymbolTable* symbols = symbol_table_new();
    AST ast = parse(&tokens, symbols);

    // Step 3: ASM Code Generation
    char* asm_src = generate_assembly(&ast, symbols);

    // Save ASM src to file and compile with NASM
    compile_asm(asm_src, compile_options);

    // Free memory
    symbol_table_free(symbols);
    preprocessor_table_free(&table);
    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);

    printf("Compilation complete\n");

    return 0;
}