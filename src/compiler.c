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
        Refactor struct type handling:
            Currently we sometimes use cast_type and sometimes use
            struct_type for structs. Unify this

    Functions:
        Variadic function definitions, not just declarations/calling:
            https://blog.nelhage.com/2010/10/amd64-and-va_arg/
            Just recreate that struct with va_start, push the registers to stack, pass it by pointer  
            Compare with godbolt output

    Implement __LINE__ and __FILE__?

    Known issue:
        Floating point parameters + struct by value won't work, I don't save the xmm regs

    The struct padding is off by 8 bytes for ValueLabel, why?

    Issue:
        String test is checking against uninitialized values, check why

    Super weird issue:
        In pointers/ex6.c, for some very weird reason, we get segmentation fault if we
        don't include the printf function definition. It shouldn't have any code effect, but it does
        How???

    Communicate debug info down into the assembly:
        My codegen needs to know which line is responsible for what
        Maybe just include line info in the ASTNode.
        Then the Token communicates the original line content by
        initializing at runtime

    Types:
        Arrays:
            Infer array size from initializer size
            Treat x[] as *x
            These are connected. int x[] is equivalent with int* x;
            char* x = {"a", "b", "c"} should work if char x[] works, same construct
            Variable length arrays: just add to rsp, then set address to this

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
    tokens_free_line_strings(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(compile_options.output_filename);

    printf("Compilation complete\n");

    return 0;
}