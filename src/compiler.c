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

    Structs:

    Typedef issue:
        Typedefs are allowed before declaration of a type
        This is problematic. The current way doesn't work for that
        How do fix this?
        Forward declaration of structs
        The reason I get in trouble for this is that I store
        the struct declaration inside the type. This causes problems,
        because the typedef doesn't contain any of this information,
        and even if I declare now I have a typedef without any info

        Several ways to fix this:
            1. I can backtrack and fix the typedefs in post. Doesn't seem very clean
            2. I can separate the struct declaration and definition. Store all of this in
                the struct object in the symbol table, which I look up in the symbol table.
                I store this info inside the Variable on object creation. I only need the name 
                struct name inside the VarType. This is a large refactor though, but seems more clean
            3. I change all code to not use typedefs like this. Seems sketch
            4. I store the type tokens inside the typedef. Then, everytime I encounter the typedef
                I treat it as the containing type tokens. This is a bit nasty, because I'd have to
                poke around in parse_token, but would probably work and be pretty short. 
                I'd also have to manually figure out what the typedef is, which shouldn't be super hard,
                just have to search for ;. accept_type() wouldn't work as 
        Let's do number 2, seems like a clean refactor all around.

        There is technically a similar problem with enums, but I don't
        keep track of enum content anyway so it's fine
        

    Types:
        Arrays:
            Infer array size from initializer size
            Treat x[] as *x
            These are connected. int x[] is equivalent with int* x;
            char* x = {"a", "b", "c"} should work if char x[] works, same construct

    Intentional deficits:
        Constants expressions are not evaluated
        Only literals are allowed for constant certain expressions
        Variable length arrays are not implemented
        Comma operator and ternary operator is not implemented
        Certain float operators have been skipped
        No function pointers or local functions

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