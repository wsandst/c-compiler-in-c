#pragma once
#include <stdbool.h>
#include "tokens.h"
#include "string.h"

typedef enum VarTypeEnum VarTypeEnum;
typedef enum BinaryOpType BinaryOpType;
typedef enum UnaryOpType UnaryOpType;
typedef enum ASTNodeType ASTNodeType;
typedef struct Variable Variable;
typedef struct Function Function;
typedef struct ASTNode ASTNode;
typedef struct AST AST;

enum VarTypeEnum {
  TY_VOID,
  TY_BOOL,
  TY_CHAR,
  TY_SHORT,
  TY_INT,
  TY_LONG,
  TY_FLOAT,
  TY_DOUBLE,
  TY_ENUM,
  TY_PTR,
  TY_STRUCT,
  //TY_UNION,
  //TY_ARRAY,
  //TY_FUNC,
};

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
    AST_BIN_OP,
    AST_UNARY_OP,
    //AST_COMMA,    // ,
    //AST_MEMBER,   // . (struct member access)
    AST_RETURN,     // "return"
    AST_IF,         // "if"
    AST_FOR,        // "for" or "while"
    AST_DO,         // "do"
    AST_SWITCH,     // "switch"
    AST_CASE,       // "case"
    AST_BLOCK,      // { ... }
    //AST_GOTO,       // "goto"
    //AST_LABEL,      // Labeled statement
    AST_FUNC_CALL,  // Function call
    AST_FUNC,
    AST_STMT,
    AST_VAR,        // Variable
    AST_NUM,        // Integer
    AST_CAST,       // Type cast
    AST_ASSIGN, // This is actually an operation in C. Start out like this?
};

struct Variable {
    Variable *next;
    char* name;
    VarTypeEnum type;
    int size;
    // Need to represent scope somehow
};

// Functions are stored outside of the main AST
struct Function {
    char* name;
    VarTypeEnum return_type;
    VarTypeEnum* params;
    ASTNode *body;

    Function *next_mem; // Linked list used for freeing memory correctly
};

struct ASTNode {
    ASTNodeType type;
    Variable var;
    Function* func; // Call
    ASTNode *func_args;

    ASTNode *next;

    // OP
    BinaryOpType bop_type;
    UnaryOpType uop_type;
    ASTNode *rhs;
    ASTNode *lhs;

    // IF, WHILE, FOR etc
    ASTNode *cond;
    ASTNode *then;
    ASTNode *els;
    // For
    ASTNode *incr;
    ASTNode *init;

    //ASTNode *brk;
    //ASTNode *cont;

    // Function
    ASTNode *args;
    VarTypeEnum ret_type;
    ASTNode *ret; // necessary?
    // return?

    ASTNode *body;
    // Literal
    long long int ival;
    long double fval;

    char *debug;
    ASTNode *next_mem; // Linked list used for freeing memory correctly
};

struct AST {
    ASTNode *program;
    Function *functions; // Hashmap here as well
    Variable *variables; // Hashmap probably? Or map to integers
};

// AST functionality

// Free the memory of the AST
void ast_free(AST *ast);

ASTNode *ast_node_new(ASTNodeType type, int count);

void ast_node_free(ASTNode *ast_node);

Function *function_new(char* name);

void function_free(Function *func);

// Take in a list of tokens and return an Abstract Syntax Tree
AST parse(Tokens *tokens);

int find_main_index(Tokens *tokens);

void parse_decend(Tokens *tokens, int token_index, ASTNode *node);

void parse_decend_func(Tokens *tokens, int token_i, ASTNode *node);

VarTypeEnum token_type_to_var_type(enum TokenType type);