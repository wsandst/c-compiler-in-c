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
        Constant expressions (compile time expressions) (only required for advanced switch cases)

    Function parameters, proper calling convention:
        Start with adding one parameter
        Return 0 from functions if there is no return statement, won't follow spec otherwise

    Loops: Complete?

    Switch statements:
        They have to be constant expressions
        switch(x) {
            case 1:
            case 2:
            case 3:
            case 4:
        }
        case 1: is a statement, just as much as x = 4;
        case 1: code ... is just a label: code in asm
        The switch has code to determine which label to jump
        Nothing stopping us from doing duffs device

        How does the switch know the labels of the cases?
        Two types of labels:
        Labels with indentifiers (per function)
        Labels with values (like in switch). These need to be represented somehow
        Add labels to the symbol map
        A switch can get 

        What do I want?
        I want an AST Node Switch, which has a body
        this body contains statements, some of which are labels with values
        These labels need unique identification
        The switch needs to know all of these labels, in an array perhaps, for codegen

        My symbol map will track labels. I need to create a new scope for every new switch.
        Then, once the body of a switch is parsed, I use the symbol map to get every label below this one
        (unless I hit a switch scope, signalling nested switches). I can then store this in the switch

    Gotos: 
        I need to do two passes to ensure the labels actually exist
        I could also just always assume they do exist, and just save it in the AST
        You could goto between functions in that case, which is a bit weird

    Symbol map:
        Track labels
        Track typedefs

    Globals

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
    SymbolTable* symbols = symbol_table_new();
    AST ast = parse(&tokens, symbols); 

    // Step 3: ASM Code Generation
    char* asm_src = generate_assembly(&ast);

    compile_asm(asm_src);

    // Free memory
    symbol_table_free(symbols);
    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    free(src);
    
    printf("Compilation complete\n");

    return 0;
}