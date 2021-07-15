/*
Take in a source file as a string and return an array of TOKENS
*/
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_helpers.h"

enum TokenType {TK_NONE, TK_OP, TK_IDENT, TK_TYPE, TK_KEYWORD, TK_LITERAL, TK_DELIMITER, TK_COMMENT, TK_PREPROCESSOR,
            TK_LINT, TK_LFLOAT, TK_LSTRING, TK_LCHAR};

enum OpType {OP_PLUS, OP_MINUS, OP_EXP, OP_MULT, OP_DIV, OP_MOD, OP_RIGHTSHIFT, OP_LEFTSHIFT, OP_BITOR, OP_BITAND,
        OP_COMPL, OP_XOR, OP_NOT, OP_AND, OP_OR, OP_EQ, OP_NEQ, OP_ASSIGN, OP_GT, OP_LT, OP_LTE, OP_GTE, OP_QST};

enum KeywordType {KW_IF, KW_ELSE, KW_WHILE, KW_DO, KW_FOR, KW_BREAK, KW_CONTINUE, KW_RETURN, KW_SWITCH, KW_CASE, KW_DEFAULT, KW_GOTO,
            KW_LABEL, KW_TYPEDEF, KW_INCLUDE, KW_DEFINE, KW_CONST, KW_LONG, KW_SHORT, KW_SIGNED, KW_UNSIGNED, KW_STRUCT, KW_UNION, 
            KW_INT, KW_FLOAT, KW_DOUBLE, KW_CHAR, KW_VOID};

enum DelimType {DL_SEMICOLON, DL_COMMA, DL_COLON, DL_DOT, DL_OPENPAREN, DL_CLOSEPAREN, 
    DL_OPENBRACE, DL_CLOSEBRACE, DL_OPENBRACKET, DL_CLOSEBRACKET};

typedef struct Token Token;
typedef struct Tokens Tokens;

struct Token {
    enum TokenType type;
    union value {
        char* string;
        int ivalue;
        float fvalue;
        double dvalue;
        char cvalue;
        enum KeywordType keyword;
        enum OpType op;
        enum DelimType delim;
    } value;
    int src_pos;
    char* string_repr;
};

struct Tokens {
    Token* elems;
    int size;
};

// Create a new Tokens object
Tokens tokens_new(int size);

// Free the Token object
void tokens_free(Tokens *tokens);

// Remove NULL elements from the token array
void tokens_trim(Tokens* tokens);

void tokens_print(Tokens* tokens);

Tokens tokenize(char* source);

void tokenize_preprocessor(Tokens *tokens, StrVector *split_src);

void tokenize_comments(Tokens *tokens, StrVector *str_split);

void tokenize_strings(Tokens *tokens, StrVector *str_split);
void tokenize_chars(Tokens *tokens, StrVector *str_split);

void tokenize_keywords(Tokens *tokens, StrVector *str_split);
void tokenize_keyword(Tokens* tokens, StrVector *str_split, char* keyword, enum KeywordType type);

void tokenize_ops(Tokens *tokens, StrVector *str_split);
void tokenize_op(Tokens* tokens, StrVector *str_split, char* op, enum OpType type);

void tokenize_idents(Tokens *tokens, StrVector *str_split);

void tokenize_delims(Tokens *tokens, StrVector *str_split);

void tokenize_values(Tokens *tokens, StrVector *str_split);
