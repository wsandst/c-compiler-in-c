#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"

/*
TODO:
    Expression overhaul:
        I need to start thinking about expression scratch space,
        and cleanup expression handling in general
        There are four types of expressions:
        Literals, variables, unary ops and binary ops
        Every leaf node has to be either a literal or a variable
        Step 1: Implement unary ops
        Step 2: Implement binary ops
        Step 3: Implement assignment as binary op
        


    Simple unary ops, then binary ops
    Function parameters, proper calling convention
    If conditionals
    While loops
    For loops
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
    AST ast = parse(&tokens); 

    // Step 3: ASM Code Generation
    //char* asm_src = load_file_to_string("../test/resources/hello_world.asm");
    char* asm_src = generate_assembly(&ast);

    compile_asm(asm_src);

    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(src);
    
    printf("Compilation complete\n");

    return 0;
}