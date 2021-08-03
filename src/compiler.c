#include "preprocess.h"
#include "parser.h"
#include "codegen.h"

/*
TODO:
    Next time:
        Start investigating type system

    Expressions:
        Constant expressions (compile time expressions) (only really required for advanced switch cases):
            I can do this as a post-processing step on the parse tree, or perhaps in the binary/unary
            op handling.

    Functions, proper calling convention:
        Stack arguments, floating point arguments, floating point returns

    Types:
        Type conversion:
            bool -> char -> short int -> int -> 
            unsigned int -> long -> unsigned -> 
            long long -> float -> double -> long double
        Arrays
        sizeof:
            I need some way of isolating casts. sizeof basically acts on a cast
            Currently the system will just eat the parenthesis, find the type,
            be confused. Lookahead after parenthesis maybe? I still need to handle
            *type, but that is essentially also a type
            Alright, so: Upon encountering a parenthesis in an expression, we 
            perform lookahead for a type (value type, pointer type, struct etc)
            If this is a type, this node is an unary node of type UOP_CAST, and the
            type is stored somewhere. Make a accept_unop_cast() or something
            The one exception to this is if we encounter a sizeof unary operator. We need
            to manually look for a sizeof (TYPE) (with accept_unop_cast()), as it is a literal
            of sorts now, not an unary op.
            Supported byte sizes: 
                Integers:
                    char (1 byte), short int/short (2 bytes), int (4 bytes), long int/long (8 bytes)
                Floats:
                    float (4 bytes), double (8 bytes)
                Pointers:
                    Always 8 bytes, but increment is based on the representing type

    Preprocessor:
        Defines:
            Simple defines, ifndef, ifdef. Goal is to support simple include headers
            and simple constant defines.
        Includes:
            Support STD headers. I should take from GCC and remove all the macros
    
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

    compile_asm(asm_src);

    // Free memory
    symbol_table_free(symbols);
    tokens_free(&tokens);
    ast_free(&ast);
    free(asm_src);
    
    printf("Compilation complete\n");

    return 0;
}