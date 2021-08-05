/*
These functions perform parsing of C source code. 
This is the second step of a compiler. The tokens from the tokenization step
are parsed and turned into an Abstract Syntax Tree (AST).
This is done through a recursive descent parser,
which is implemented here.
*/
#pragma once
#include <stdbool.h>
#include <string.h>
#include "tokens.h"
#include "symbol_table.h"

typedef enum OpType OpType;
typedef enum UnaryOpType UnaryOpType;
typedef enum ASTNodeType ASTNodeType;
typedef enum ExprType ExprType;
typedef struct ASTNode ASTNode;
typedef struct AST AST;

enum OpType {
    BOP_ADD,                // +
    BOP_SUB,                // -
    BOP_MUL,                // *
    BOP_DIV,                // /
    BOP_MOD,                // %
    BOP_BITAND,             // &
    BOP_BITOR,              // |
    BOP_BITXOR,             // ^
    BOP_LEFTSHIFT,          // <<
    BOP_RIGHTSHIFT,         // >>
    BOP_EQ,                 // ==
    BOP_NEQ,                // !=
    BOP_LT,                 // <
    BOP_LTE,                // <=
    BOP_GT,                 // <
    BOP_GTE,                // <=
    BOP_AND,                // &&
    BOP_OR,                 // ||
    BOP_ASSIGN,             // =
    BOP_ASSIGN_ADD,         // +=
    BOP_ASSIGN_SUB,         // -=
    BOP_ASSIGN_MULT,        // *=
    BOP_ASSIGN_DIV,         // /=
    BOP_ASSIGN_MOD,         // %=
    BOP_ASSIGN_RIGHTSHIFT,  // >>=
    BOP_ASSIGN_LEFTSHIFT,   // <<=
    BOP_ASSIGN_BITAND,      // &=
    BOP_ASSIGN_BITOR,       // |=
    BOP_ASSIGN_BITXOR,      // ^=
    UOP_NEG,                // unary -
    UOP_ADDR,               // unary &
    UOP_DEREF,              // unary *
    UOP_NOT,                // unary !
    UOP_COMPL,              // unary ~
    UOP_SIZEOF,             // sizeof
    UOP_POST_INCR,          // x++
    UOP_PRE_INCR,           // ++x
    UOP_POST_DECR,          // x--
    UOP_PRE_DECR,           // --x
    UOP_CAST,
};

enum ASTNodeType {
    AST_PROGRAM,
    AST_EXPR,
    AST_SCOPE,      // { ... }, scope

    AST_FUNC,       // Function def
    AST_RETURN,     // Return
    AST_IF,         // If
    AST_LOOP,       // While, for
    AST_DO_LOOP,    // Do while
    AST_BREAK,      // Break
    AST_CONTINUE,   // Continue
    AST_SWITCH,     // Switch
    AST_CASE,       // Case
    AST_GOTO,       // Goto
    AST_LABEL,      // Label
    //AST_COMMA,    // ,

    AST_STMT,       // Statement
    AST_END,        // End of scope/loop etc
    AST_NULL_STMT, // Empty statement, used in certain for loops, need to represent it
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
    ValueLabel label;

    ASTNode* next;
    ASTNode* body;

    // Expr
    ExprType expr_type;
    OpType op_type;
    VarType cast_type;
    ASTNode* rhs;
    ASTNode* lhs;
    // Literal
    char* literal;
    bool top_level_expr;

    // IF, WHILE, FOR etc
    ASTNode* cond;
    ASTNode* els;
    ASTNode* incr;

    ValueLabel* switch_cases; // Used for switch cases

    // Function
    ASTNode* args;
    VarTypeEnum ret_type;
    ASTNode* ret; // necessary?

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

// Swap the memory of two nodes
void ast_node_swap(ASTNode* node1, ASTNode* node2);

// Copies node2 into node1
void ast_node_copy(ASTNode* node1, ASTNode* node2);

// =========== Parsing ============
// Uses recursive decending to construct the AST

// Take in a list of tokens and return an Abstract Syntax Tree
AST parse(Tokens* tokens, SymbolTable* global_symbols);

// Parse the program (file)
// <program> ::= { <function> | <declaration> }
void parse_program(ASTNode* node, SymbolTable* symbols);

// Parse a function definition
// <function> ::= <type> <id> "(" <args ...> ")" <scope>
void parse_func(ASTNode* node,  SymbolTable* symbols);

// Parse a statement, continue with next statement
// <statement> ::= <keyword> | <expression> | <declaration>
void parse_statement(ASTNode* node,  SymbolTable* symbols);

// Parse a single statement, do not continue with next.
void parse_single_statement(ASTNode* node, SymbolTable* symbols);

// Parse an expression, ex (a + b) + 3
void parse_expression(ASTNode* node,  SymbolTable* symbols, int min_precedence);
void parse_expression_atom(ASTNode* node,  SymbolTable* symbols);
// Used later for short circuiting
void mark_first_and_or_nodes(ASTNode* node, OpType new_op);

void parse_global(ASTNode* node, SymbolTable* symbols);

// <function_call> ::= <identifier> "(" <expression ...> ")"
void parse_func_call(ASTNode* node,  SymbolTable* symbols);

// Parse a scope/block
// <scope> ::= "{" <statement ...> "}"
void parse_scope(ASTNode* node, SymbolTable* symbols);

// Parse an if conditional
// <if> ::= "if" "(" <expression> ")" <statement>
void parse_if(ASTNode* node, SymbolTable* symbols);

// Parse a while loop
// <while> ::= "while" "(" <expression> ")" <statement>
void parse_while_loop(ASTNode* node, SymbolTable* symbols);

// Parse a do while loop
// <do_while> ::= "do" <statement> "while" "(" <expression> ")"
void parse_do_while_loop(ASTNode* node, SymbolTable* symbols);

// Parse a for loop
// <for> ::= "for" "(" <statement> <expression> <expression> ")" <statement>
void parse_for_loop(ASTNode* node, SymbolTable* symbols);

// Parse a switch statement
// <switch> ::= "switch" "(" <expression> ")" (<statement> | <case> | <default_case>)
void parse_switch(ASTNode* node, SymbolTable* symbols);

// Parse a case statement
// <case> ::= "case" <identifier> ":"
void parse_case(ASTNode* node, SymbolTable* symbols);

// Parse a default case statement
// <default_case> ::= "default" ":"
void parse_default_case(ASTNode* node, SymbolTable* symbols);

// Print a parse error to stderr and exit the program
void parse_error(char* error_message);
void parse_error_unexpected_symbol(enum TokenType expected, enum TokenType recieved);

bool is_const_expression(ASTNode* node, SymbolTable* symbols);

// Evaluate a constant expression
char* evaluate_const_expression(ASTNode* node, SymbolTable* symbols);

// Various helpers

// Return the previous token parsed
Token prev_token();

// Make sure the current token matches the sent in token, else
// send an error message and exit the program
void expect(TokenType type);

// Return true or false whether the current token matches the sent in token
bool accept(TokenType type);
// Accept any token within this range
bool accept_range(TokenType from_token, TokenType to_token);
// Accept a unary operator Token type
bool accept_unop_type();
// Accept postfix/suffix unary operator Token Type
bool accept_post_unop();
// Accept a binary operator Token type
bool accept_binop_type();
// Accept a variable/fd type
bool accept_type();

// Get precedence of binary operator
int get_binary_operator_precedence(OpType type);
// Assignment is right associative, needed for the precedence parsing
bool is_binary_operation_assignment(OpType type);

// Return the wider variable type.
// char > short > int > long > float > double > long double
VarType return_wider_type(VarType type1, VarType type2);

// Convert a TokenType unary operator type to the corresponding prefix UnaryOpType
OpType token_type_to_pre_uop_type(TokenType type);
// Convert a TokenType unary operator type to the corresponding postfix UnaryOpType
OpType token_type_to_post_uop_type(TokenType type);
// Convert a TokenType binary operator type to the corresponding BinaryOpType
OpType token_type_to_bop_type(TokenType type);