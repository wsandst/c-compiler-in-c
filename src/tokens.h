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

#include "util/vector.h"
#include "util/string_helpers.h"

// Represents different token types in the C language
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
    TK_OP_PTR_MEMBER,
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
    TK_KW_TYPEDEF,
    TK_KW_INCLUDE,
    TK_KW_DEFINE,
    TK_KW_CONST,
    TK_KW_LONG,
    TK_KW_SHORT,
    TK_KW_SIGNED,
    TK_KW_UNSIGNED,
    TK_KW_EXTERN,
    TK_KW_STATIC,
    TK_KW_STRUCT,
    TK_KW_ENUM,
    TK_KW_UNION,
    TK_KW_INT,
    TK_KW_FLOAT,
    TK_KW_DOUBLE,
    TK_KW_CHAR,
    TK_KW_VOID,
    TK_KW_VARIADIC_DOTS,
};

typedef enum TokenType TokenType;

// Represents a token object with various useful information
struct Token {
    enum TokenType type;
    int src_pos;
    int src_line;
    // Further token origin information used for error messages and debugging
    char* string_repr;
    char* src_filename;
    char* src_line_str;
    bool requires_string_free;
    bool src_line_requires_free;
};

// Represents a vector of tokens
struct Tokens {
    int size;
    Vec elems; // Token vec
    StrVector line_string_vec; // Used for freeing
};

typedef struct Token Token;
typedef struct Tokens Tokens;

// ========= Tokens object functionality ===========

// Create a new Tokens object
Tokens tokens_new(int size);

// Free the Tokens object
void tokens_free(Tokens* tokens);

// Free the Tokens line strings
// This is separate, as we do not always want to free this
void tokens_free_line_strings(Tokens* tokens);

// Get a token from index i in Tokens
Token* tokens_get(Tokens* tokens, int i);

// Set a token at index i in Tokens
void tokens_set(Tokens* tokens, int i, TokenType type, char* string_repr,
                bool requires_free);

// Remove NULL elements from the token array
void tokens_trim(Tokens* tokens);

// Necessary to free correctly with tokens_free in some cases
Tokens tokens_copy(Tokens* tokens);

void tokens_tag_src_filename(Tokens* tokens, char* filename);

// Print a Tokens object
void tokens_print(Tokens* tokens);

// Pretty print of a Tokens object, approximates the original source code.
void tokens_pretty_print(Tokens* tokens);

// Insert the entire tokens2 into tokens1 at a specific index in tokens1
Tokens* tokens_insert(Tokens* tokens1, Tokens* tokens2, int tokens1_index);

// ========= Tokenization functions ===========

// Convert a source string into a tokens object
Tokens tokenize(char* source, bool tag_debug_line_info);

// Tokenize preprocessor tokens
void tokenize_preprocessor(Tokens* tokens, StrVector* split_src);

// Tokenize comments
void tokenize_comments(Tokens* tokens, StrVector* str_split);

// Tokenize strings
void tokenize_strings(Tokens* tokens, StrVector* str_split);
// Tokenize characters
void tokenize_chars(Tokens* tokens, StrVector* str_split);

// Tokenize keywords
void tokenize_keywords(Tokens* tokens, StrVector* str_split);
// Helper for tokenize_keywords, tokenize a specific keyword
void tokenize_keyword(Tokens* tokens, StrVector* str_split, char* keyword,
                      enum TokenType type);

void tokenize_ops(Tokens* tokens, StrVector* str_split);
// Helper for tokenize_ops, tokenize a specific op
void tokenize_op(Tokens* tokens, StrVector* str_split, char* op, enum TokenType type);

// Tokenize all identifiers
void tokenize_idents(Tokens* tokens, StrVector* str_split);

// Tokenize all literal values
void tokenize_values(Tokens* tokens, StrVector* str_split);
// Helpers for tokenize_values()
void tokenize_ints(Tokens* tokens, StrVector* str_split);
void tokenize_floats(Tokens* tokens, StrVector* str_split);

// Tokenize delimiters
void tokenize_delims(Tokens* tokens, StrVector* str_split);

// Tag tokens with their original source line
void tag_tokens_with_src_lines(Tokens* tokens, StrVector* str_split);

// Debug helper
char* token_type_to_string(enum TokenType type);