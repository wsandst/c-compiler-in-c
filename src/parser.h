/*
These functions perform parsing of C source code. 
This is the second step of a compiler. The tokens from the tokenization step
are parsed and turned into an Abstract Syntax Tree (AST).
This is done through a recursive descent parser,
which is implemented here.
*/
#pragma once
#include <stdbool.h>
#include "tokens.h"
#include "string.h"
#include "symbol_table.h"

typedef enum BinaryOpType BinaryOpType;
typedef enum UnaryOpType UnaryOpType;
typedef enum ASTNodeType ASTNodeType;
typedef enum ExprType ExprType;
typedef struct ASTNode ASTNode;
typedef struct AST AST;

enum BinaryOpType {
    BOP_ADD,        // +
    BOP_SUB,        // -
    BOP_MUL,        // *
    BOP_DIV,        // /
    BOP_MOD,        // %
    BOP_BITAND,     // &
    BOP_BITOR,      // |
    BOP_BITXOR,     // ^
    BOP_LEFTSHIFT,  // <<
    BOP_RIGHTSHIFT, // >>
    BOP_EQ,         // ==
    BOP_NE,         // !=
    BOP_LT,         // <
    BOP_LE,         // <=
    BOP_ASSIGN,     // =
    BOP_AND,        // &&
    BOP_OR,         // ||
};

enum UnaryOpType {
    UOP_NEG,        // unary -
    UOP_ADDR,       // unary &
    UOP_DEREF,      // unary *
    UOP_NOT,        // unary !
    UOP_COMPL,      // unary ~
    UOP_SIZEOF,     // should this be considered an op?
};

enum ASTNodeType {
    AST_PROGRAM,
    AST_EXPR,
    //AST_COMMA,    // ,
    //AST_MEMBER,   // . (struct member access)
    AST_RETURN,     // "return"
    AST_IF,         // "if"
    AST_FOR,        // "for" or "while"
    AST_DO,         // "do"
    AST_SWITCH,     // "switch"
    AST_CASE,       // "case"
    AST_BLOCK,      // { ... }
    //AST_GOTO,     // "goto"
    //AST_LABEL,    // Labeled statement
    AST_FUNC,
    AST_STMT,
    AST_VAR_DEC,    // Variable declaration
    AST_CAST,       // Type cast
    AST_ASSIGN,     // This is actually an operation in C. Start out like this?
    AST_END,
    AST_NONE,
};

enum ExprType {
    EXPR_BINOP,
    EXPR_UNOP,
    EXPR_LITERAL,
    EXPR_VAR,
    EXPR_FUNC_CALL,
};

struct ASTNode {
    ASTNodeType type;
    Variable var;
    Function func;

    ASTNode* func_args;

    ASTNode* next;

    // Assign
    ASTNode* assign;

    // Expr
    ExprType expr_type;
    BinaryOpType bop_type;
    UnaryOpType uop_type;
    ASTNode* rhs;
    ASTNode* lhs;
    // Literal
    char* literal;

    // IF, WHILE, FOR etc
    ASTNode* cond;
    ASTNode* then;
    ASTNode* els;
    // For
    ASTNode* incr;
    ASTNode* init;

    //ASTNode *brk;
    //ASTNode *cont;

    // Function
    ASTNode* args;
    VarTypeEnum ret_type;
    ASTNode* ret; // necessary?
    // return?

    ASTNode* body;
    //long long int ival;
    //long double fval;

    char* debug;
    ASTNode* next_mem; // Linked list used for freeing memory correctly
};

struct AST {
    ASTNode* program;
    Function* functions; // Hashmap here as well
    Variable* variables; // Hashmap probably? Or map to integers
};

// =========== AST struct functionality ============

// Destructor, free the memory of the AST
void ast_free(AST* ast);

// Constructor, create new AST node
ASTNode *ast_node_new(ASTNodeType type, int count);

// Destructor, free the AST node
void ast_node_free(ASTNode* ast_node);

// =========== Parsing ============
// Uses recursive decending to construct the AST

// Take in a list of tokens and return an Abstract Syntax Tree
AST parse(Tokens* tokens);

// Parse the program (file)
void parse_program(ASTNode* node, SymbolTable* symbols);

// Parse a function definition
// <function> ::= <type> <id> "(" <args> ")" "{" <statement> "}"
void parse_func(ASTNode* node,  SymbolTable* symbols);

// Parse a statement
void parse_statement(ASTNode* node,  SymbolTable* symbols);

// Parse an expression, ex (a + b) + 3
void parse_expression(ASTNode* node,  SymbolTable* symbols);

// Parse a scope/block
void parse_scope(ASTNode* node, SymbolTable* symbols);

// Print a parse error to stderr and exit the program
void parse_error(char* error_message);
void parse_error_unexpected_symbol(enum TokenType expected, enum TokenType recieved);

// Various helpers

// Return the previous token parsed
Token prev_token();

// Make sure the current token matches the sent in token, else
// send an error message and exit the program
void expect(enum TokenType type);
void expect_var_type();

// Return true or false whether the current token matches the sent in token
bool accept(enum TokenType type);
bool accept_var_type();

// Find the index of the main function
int find_main_index(Tokens* tokens);

// Convert a TokenType variable type to the corresponding VarTypeEnum
VarTypeEnum token_type_to_var_type(enum TokenType type);
// Convert a TokenType unary operator type to the corresponding UnaryOpType
UnaryOpType token_type_to_unop_type(enum TokenType type);