#pragma once
#include <stdbool.h>
#include "tokens.h"

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
    AST_STMT,
    AST_VAR,        // Variable
    AST_NUM,        // Integer
    AST_CAST,       // Type cast
};

struct VarType {
    VarTypeEnum type;
    int size; //sizeof()
    bool is_unsigned;
};

struct Variable {
    Variable *next;
    char* name;
    VarTypeEnum type;
};

// Functions are stored outside of the main AST

struct Function {
    Function *next;
    char* name;
    VarTypeEnum return_type;
    VarTypeEnum* params;
    ASTNode *body;
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
    // return?

    ASTNode *body;
    // Literal
    long long int ival;
    long double fval;
};

struct AST {
    ASTNode program;
    Function *functions;
    Variable *variables;
};

// AST functionality
// Create a new AST
AST ast_new();

// Free the memory of the AST
void ast_free(AST *ast);


// Take in a list of tokens and return an Abstract Syntax Tree
AST parse(Tokens tokens);