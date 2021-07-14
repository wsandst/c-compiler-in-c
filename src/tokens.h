/*
Take in a source file as a string and return an array of TOKENS
*/
#pragma once
#include <stdlib.h>
#include <string.h>
#include "string_helpers.h"

enum TokenType {TK_NONE, TK_OP, TK_IDENT, TK_TYPE, TK_KEYWORD, TK_LITERAL, TK_DELIMITER, TK_COMMENT, TK_PREPROCESSOR};

enum LiteralType {TK_LINT, TK_LFLOAT, TK_LSTRING, TK_LCHAR};

enum OpType {TK_PLUS, TK_MINUS, TK_EXP, TK_MULT, TK_DIV, TK_MOD, TK_RIGHTSHIFT, TK_LEFTSHIFT, TK_BITOR, TK_BITAND,
        TK_COMPL, TK_XOR, TK_NOT, TK_AND, TK_OR, TK_EQ, TK_NEQ, TK_ASSIGN, TK_GT, TK_LT, TK_LTE, TK_GTE, TK_QST};

enum KeywordType {TK_IF, TK_ELSE, TK_WHILE, TK_DO, TK_FOR, TK_BREAK, TK_CONTINUE, TK_RETURN, TK_SWITCH, TK_CASE, TK_DEFAULT, TK_GOTO,
            TK_LABEL, TK_TYPEDEF, TK_INCLUDE, TK_DEFINE, TK_CONST, TK_LONG, TK_SHORT, TK_SIGNED, TK_UNSIGNED, TK_STRUCT, TK_UNION};

enum DelimType {TK_SEMICOLON, TK_COMMA, TK_COLON, TK_DOT, TK_OPENPAREN, TK_CLOSEPAREN, 
    TK_OPENBRACE, TK_CLOSEBRACE, TK_OPENBRACKET, TK_CLOSEBRACKET};

enum ValueType {TK_INT, TK_FLOAT, TK_DOUBLE, TK_CHAR, TK_VOID};

typedef struct Token Token;
typedef struct Tokens Tokens;

struct Token {
    enum TokenType type;
    union sub_type {
        enum KeywordType keyword;
        enum OpType op;
        enum DelimType delim;
        enum ValueType value;
        enum LiteralType literal;
    } sub_type;
    union data {
        char* string;
        int ivalue;
        float fvalue;
        double dvalue;
        char cvalue;
    } data;
    int src_pos;
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
void tokenize_types(Tokens *tokens, StrVector *str_split);
void tokenize_ops(Tokens *tokens, StrVector *str_split);

void tokenize_idents(Tokens *tokens, StrVector *str_split);

void tokenize_delims(Tokens *tokens, StrVector *str_split);

void tokenize_values(Tokens *tokens, StrVector *str_split);
