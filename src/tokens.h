/*
Take in a source file as a string and return an array of TOKENS
*/

#include <stdlib.h>

enum TokenType {OP, IDENT, TYPE, KEYWORD, DELIMITER, COMMENT, PREPROCESSOR};

enum OpType {PLUS, MINUS, EXP, MULT, DIV, MOD, RIGHTSHIFT, LEFTSHIFT, BITOR, BITAND,
        COMPL, XOR, NOT, AND, OR, EQ, NEQ, ASSIGN, GT, LT, LTE, GTE, QST};

enum KeywordType {IF, ELSE, WHILE, DO, FOR, BREAK, CONTINUE, RETURN, SWITCH, CASE, DEFAULT, GOTO,
            LABEL, TYPEDEF, INCLUDE, DEFINE, CONST, LONG, SHORT, SIGNED, UNSIGNED, STRUCT, UNION};

enum DelimType {SEMICOLON, COMMA, COLON, DOT, OPENPAREN, CLOSEPAREN, OPENBRACE, CLOSEBRACE, OPENBRACKET, CLOSEBRACKET};

enum ValueType {INT, FLOAT, DOUBLE, CHAR, VOID};

typedef struct Token Token;
typedef struct Tokens Tokens;

struct Token {
    enum TokenType type;
    union sub_type {
        enum KeywordType keyword;
        enum OpType op;
        enum DelimType delim;
        enum ValueType value;
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
    Token* token_array;
    int size;
    int max_size;
};

Tokens tokenize(char* source);

// Create a new Tokens object
Tokens tokens_new(int initial_size);

// Add a token object to the end of the vector 
void tokens_push(Tokens *tokens, Token token);

// Sort the tokens based on the src_pos attribute
void tokens_sort(Tokens *tokens);

void tokens_free(Tokens *tokens);