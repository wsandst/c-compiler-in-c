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
        Refactor function calling
        Refactor codegen, decrease unnecessary assembly:
            Leaf nodes vs non-leaf nodes
            It is unneccessary to push on leaf nodes

    Functions, proper calling convention:
        Variadic function definitions, not just declarations/calling:
            https://blog.nelhage.com/2010/10/amd64-and-va_arg/
            Just recreate that struct with va_start, push the registers to stack, pass it by pointer
        Alignment issue:
            My rsp must be 16-byte aligned at function call
            This is only an issue for external function calls, but
            I should fix it to adhere to the calling convention
            How do I respect it though? 
            I need to adjust before function call, which
            messes up the stack arguments unless I adjust before
            pushing them. But then I need forward knowledge
        

    Typedef issue:
        Does the current way respect alignment? Not sure        

    Types:
        Arrays:
            Infer array size from initializer size
            Treat x[] as *x
            These are connected. int x[] is equivalent with int* x;
            char* x = {"a", "b", "c"} should work if char x[] works, same construct
            Variable length arrays: just add to rsp, then set address to this
    
    Issue:
        Certain binary operations ([], ., ->) need to have higher priority
        than unary operators. Move their check to inside or before evaluate atom maybe?


    rvalues vs lvalues:
        Complicated to implement correctly. Currently the
        compiler stores the value in rax and the address in r12
        The current issue is the deref operator: 
        Once I index a value, where does the address go?

    Intentional deficits:
        Constants expressions are not evaluated
        Only literals are allowed for constant certain expressions
        Variable length arrays are not implemented
        Comma operator and ternary operator is not implemented
        Certain float operators have been skipped
        No function pointers or local functions
        Floats are 64 bit, not 32 bit. Messes with scanf

*/

int main(int argc, char** argv) {
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
    PreprocessorTable table = preprocessor_table_new();
    Tokens tokens = preprocess_first(src_path, &table);
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
    preprocessor_table_free(&table);
    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);

    printf("Compilation complete\n");

    return 0;
}