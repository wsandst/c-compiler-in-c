#include "tokens.h"
#include "parser.h"
#include "codegen.h"
#include "file_helpers.h"

/*
TODO:
    Expressions:
        Proper left-to-right evaluation, it doesn't work like that currently
        Operator precedence (using Shunting Yard)
        I might be using too much scratch memory, not sure if what I'm doing
        currently is optimal

    Function parameters, proper calling convention:
        Start with adding one parameter
        Return 0 from functions if there is no return statement, won't follow spec otherwise

    Loops:
        Break, continue:
            Break, continues tells the closest loop to end/start over
            The do-while loop needs a label at the bottom so I can jump there on break
            One option - I could somehow link the break with the next node after the loop
            Problem is, in codegen if I encounter a break, I know I have to jump to the end of the current loop.
            The AST for the loop doesn't have access to the label, though, which is a big issue. How do I know the label
            names? I could store labels in the symbol table at AST-time, but that seems convoluted
            One way to do it is to not link anything and instead keep track of the last label.
            This would need to be a stack though, which is annoying. I could store the label
            inside the AST node. If I could manage to link the break to it,

    Switch statements
    Gotos


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
    char* asm_src = generate_assembly(&ast);

    compile_asm(asm_src);

    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(src);
    
    printf("Compilation complete\n");

    return 0;
}