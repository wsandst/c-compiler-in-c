#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Current:
        Improve function arg handling:
            Save on stack
        Variable keyword arguments function calling, fixes printf

        Implement static values
        Floating point logical operators

        Arrays

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
        Initial support of variable keyword arguments. Just enough to make calling printf work properly

    Types:
        Floating point:
            Operators left: ++, --, logical
        Arrays:
            Array variable type, make space on stack etc
            Initializer lists would be nice to have
        Constants:
            Just ignore the keyword for now
        Static:
            If a variable is static, treat it basically identically to global,
            except no extern. The only difference is in scoping. It behaves
            like a normal scope
            Maybe I should refactor how I treat globals. Keep the declarations in the tree perhaps?
            That way I can create globals/statics as they are encountered, not by analyzing
            the symbol tree
    
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