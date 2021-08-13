/*
These functions perform tokenization of C source code. 
This is the first step of a compiler. The semantics of the language are 
separated into the Token type. To minimize dependencies to allow for self-compilation,
the tokenization is done manually using various string helpers found in string_helpers.c
Normally a task like this is more suited to regex.
*/
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util/vector.h"
#include "util/string_helpers.h"

typedef enum TokenType TokenType;

enum TokenType {
    TK_NONE, 
    TK_IDENT, 
    TK_TYPE, 
    TK_COMMENT, 
    TK_PREPROCESSOR,
    TK_EOF,
    TK_LINT, 
    TK_LFLOAT, 
    TK_LSTRING, 
    TK_LCHAR,
    // Delimiters
    TK_DL_SEMICOLON, 
    TK_DL_COMMA, 
    TK_DL_COLON, 
    TK_DL_DOT, 
    TK_DL_OPENPAREN, 
    TK_DL_CLOSEPAREN, 
    TK_DL_OPENBRACE, 
    TK_DL_CLOSEBRACE, 
    TK_DL_OPENBRACKET, 
    TK_DL_CLOSEBRACKET,
    // Operations
    TK_OP_PLUS, 
    TK_OP_MINUS, 
    TK_OP_MULT, 
    TK_OP_DIV, 
    TK_OP_MOD, 
    TK_OP_RIGHTSHIFT, 
    TK_OP_LEFTSHIFT, 
    TK_OP_BITOR, 
    TK_OP_BITAND,
    TK_OP_BITXOR, 
    TK_OP_COMPL, 
    TK_OP_NOT, 
    TK_OP_AND, 
    TK_OP_OR, 
    TK_OP_EQ, 
    TK_OP_NEQ, 
    TK_OP_GT, 
    TK_OP_LT, 
    TK_OP_LTE, 
    TK_OP_GTE, 
    TK_OP_QST,
    TK_OP_INCR,
    TK_OP_DECR,
    TK_OP_ASSIGN, 
    TK_OP_ASSIGN_ADD,
    TK_OP_ASSIGN_SUB,
    TK_OP_ASSIGN_MULT,
    TK_OP_ASSIGN_DIV,
    TK_OP_ASSIGN_MOD,
    TK_OP_ASSIGN_RIGHTSHIFT,
    TK_OP_ASSIGN_LEFTSHIFT,
    TK_OP_ASSIGN_BITAND,
    TK_OP_ASSIGN_BITOR,
    TK_OP_ASSIGN_BITXOR,
    TK_OP_SIZEOF,
    TK_KW_IF, 
    TK_KW_ELSE, 
    TK_KW_WHILE, 
    TK_KW_DO, 
    TK_KW_FOR, 
    TK_KW_BREAK, 
    TK_KW_CONTINUE, 
    TK_KW_RETURN, 
    TK_KW_SWITCH, 
    TK_KW_CASE, 
    TK_KW_DEFAULT, 
    TK_KW_GOTO,
    TK_KW_LABEL, 
    TK_KW_TYPEDEF, 
    TK_KW_INCLUDE, 
    TK_KW_DEFINE, 
    TK_KW_CONST, 
    TK_KW_LONG, 
    TK_KW_SHORT, 
    TK_KW_SIGNED, 
    TK_KW_UNSIGNED, 
    TK_KW_STRUCT, 
    TK_KW_UNION, 
    TK_KW_INT, 
    TK_KW_FLOAT, 
    TK_KW_DOUBLE, 
    TK_KW_CHAR, 
    TK_KW_VOID
};

typedef struct Token Token;
typedef struct Tokens Tokens;

struct Token {
    enum TokenType type;
    union value {
        char* string;
        long int ivalue;
        float fvalue;
        double dvalue;
        char cvalue;
    } value;
    int src_pos;
    char* string_repr;
    bool requires_string_free;
};

struct Tokens {
    Vec elems; // Token vec
    int size;
};

// ========= Tokens object functionality ===========

// Create a new Tokens object
Tokens tokens_new(int size);

// Free the Token object
void tokens_free(Tokens *tokens);

Token* tokens_get(Tokens *tokens, int i);

// Remove NULL elements from the token array
void tokens_trim(Tokens* tokens);

// Necessary to free correctly with tokens_free in some cases
Tokens tokens_copy(Tokens* tokens);

void tokens_print(Tokens* tokens);

void tokens_pretty_print(Tokens* tokens);

// Insert the entire tokens2 into tokens1 at a specific index in tokens1
Tokens* tokens_insert(Tokens* tokens1, Tokens* tokens2, int tokens1_index);

// ========= Tokenization functions ===========

Tokens tokenize(char* source);

void tokenize_preprocessor(Tokens *tokens, StrVector *split_src);

void tokenize_comments(Tokens *tokens, StrVector *str_split);

void tokenize_strings(Tokens *tokens, StrVector *str_split);
void tokenize_chars(Tokens *tokens, StrVector *str_split);

void tokenize_keywords(Tokens *tokens, StrVector *str_split);
void tokenize_keyword(Tokens* tokens, StrVector *str_split, char* keyword, enum TokenType type);

void tokenize_ops(Tokens *tokens, StrVector *str_split);
// Helper for tokenize_ops
void tokenize_op(Tokens* tokens, StrVector *str_split, char* op, enum TokenType type);

void tokenize_idents(Tokens *tokens, StrVector *str_split);

void tokenize_values(Tokens *tokens, StrVector *str_split);
// Helpers for tokenize_values()
void tokenize_ints(Tokens *tokens, StrVector *str_split);
void tokenize_floats(Tokens *tokens, StrVector *str_split);

void tokenize_delims(Tokens *tokens, StrVector *str_split);

// Debug helper
char* token_type_to_string(enum TokenType type);