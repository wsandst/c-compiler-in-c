#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/tokens.h"

void test_tokenizer_preprocessor();
void test_tokenizer_comments();
void test_tokenizer_strings();
void test_tokenizer_keywords();
void test_tokenizer_ops();
void test_tokenizer_delims();

void test_tokenizer() {
    printf("[TEST] Running tokenizer tests...\n");

    test_tokenizer_preprocessor();
    test_tokenizer_comments();
    test_tokenizer_strings();
    test_tokenizer_keywords();
    test_tokenizer_ops();
    test_tokenizer_idents();
    test_tokenizer_values();
    test_tokenizer_delims();

    printf("[TEST] Passed tokenizer tests!\n");
}

void test_tokenizer_preprocessor() {
    // Preproccessor
    char* src = "#define\n   #include test\n";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_PREPROCESSOR);
    assert(tokens.elems[1].type == TK_PREPROCESSOR);
    assert(strcmp(tokens.elems[1].value.string, "#include test") == 0);
    assert(tokens.size == 2);
    tokens_free(&tokens);

}

void test_tokenizer_comments() {
    // Comments
    char* src = "//#define\n #define \n while // hello \n /* test */ \n while /* \n if \n */while";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_COMMENT);
    assert(strcmp(tokens.elems[1].value.string, "#define") == 0);
    assert(tokens.elems[1].type == TK_PREPROCESSOR);
    assert(tokens.elems[2].type == TK_KEYWORD);
    assert(tokens.elems[3].type == TK_COMMENT);
    assert(tokens.elems[4].type == TK_COMMENT);
    assert(tokens.elems[5].type == TK_KEYWORD);
    assert(tokens.elems[6].type == TK_COMMENT);
    assert(tokens.elems[7].type == TK_KEYWORD);
    tokens_free(&tokens);
}

void test_tokenizer_strings() {
    // Strings
    char* src = "//\"\"\n \"hello\" \n \"hello\\\"\" \n 'c' \n '\\n' '\\\"'";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_COMMENT);
    assert(tokens.elems[1].type == TK_LSTRING);
    assert(strcmp(tokens.elems[1].value.string, "hello") == 0);
    assert(tokens.elems[2].type == TK_LSTRING);
    assert(strcmp(tokens.elems[2].value.string, "hello\\\"") == 0);
    assert(tokens.elems[3].type == TK_LCHAR);
    assert(strcmp(tokens.elems[3].value.string, "c") == 0);
    assert(tokens.elems[4].type == TK_LCHAR);
    assert(strcmp(tokens.elems[4].value.string, "\\n") == 0);
    assert(tokens.elems[5].type == TK_LCHAR);
    assert(strcmp(tokens.elems[5].value.string, "\\\"") == 0);
    tokens_free(&tokens);
}

void test_tokenizer_keywords() {
    // Keywords
    char* src = "unsigned.if else while do for break continue return switch case\ndefault " 
          "goto label typedef struct union const long short signed "
          "int float double char void";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].value.keyword == KW_UNSIGNED);
    assert(tokens.elems[1].type == TK_DELIMITER);
    assert(tokens.elems[2].value.keyword == KW_IF);
    assert(tokens.elems[3].value.keyword == KW_ELSE);
    assert(tokens.elems[4].value.keyword == KW_WHILE);
    assert(tokens.elems[5].value.keyword == KW_DO);
    assert(tokens.elems[6].value.keyword == KW_FOR);
    assert(tokens.elems[7].value.keyword == KW_BREAK);
    assert(tokens.elems[8].value.keyword == KW_CONTINUE);
    assert(tokens.elems[9].value.keyword == KW_RETURN);
    assert(tokens.elems[10].value.keyword == KW_SWITCH);
    assert(tokens.elems[11].value.keyword == KW_CASE);
    assert(tokens.elems[12].value.keyword == KW_DEFAULT);
    assert(tokens.elems[13].value.keyword == KW_GOTO);
    assert(tokens.elems[14].value.keyword == KW_LABEL);
    assert(tokens.elems[15].value.keyword == KW_TYPEDEF);
    assert(tokens.elems[16].value.keyword == KW_STRUCT);
    assert(tokens.elems[17].value.keyword == KW_UNION);
    assert(tokens.elems[18].value.keyword == KW_CONST);
    assert(tokens.elems[19].value.keyword == KW_LONG);
    assert(tokens.elems[20].value.keyword == KW_SHORT);
    assert(tokens.elems[21].value.keyword == KW_SIGNED);
    assert(tokens.elems[22].value.keyword == KW_INT);
    assert(tokens.elems[23].value.keyword == KW_FLOAT);
    assert(tokens.elems[24].value.keyword == KW_DOUBLE);
    assert(tokens.elems[25].value.keyword == KW_CHAR);
    assert(tokens.elems[26].value.keyword == KW_VOID);
    tokens_free(&tokens);
}

void test_tokenizer_ops() {
    // Operations
    char* src = "|| && >> << == != ** >= <= + - * / % | & ~ ^ > < ! = ?";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].value.op == OP_OR);
    assert(tokens.elems[1].value.op == OP_AND);
    assert(tokens.elems[2].value.op == OP_RIGHTSHIFT);
    assert(tokens.elems[3].value.op == OP_LEFTSHIFT);
    assert(tokens.elems[4].value.op == OP_EQ);
    assert(tokens.elems[5].value.op == OP_NEQ);
    assert(tokens.elems[6].value.op == OP_EXP);
    assert(tokens.elems[7].value.op == OP_GTE);
    assert(tokens.elems[8].value.op == OP_LTE);
    assert(tokens.elems[9].value.op == OP_PLUS);
    assert(tokens.elems[10].value.op == OP_MINUS);
    assert(tokens.elems[11].value.op == OP_MULT);
    assert(tokens.elems[12].value.op == OP_DIV);
    assert(tokens.elems[13].value.op == OP_MOD);
    assert(tokens.elems[14].value.op == OP_BITOR);
    assert(tokens.elems[15].value.op == OP_BITAND);
    assert(tokens.elems[16].value.op == OP_COMPL);
    assert(tokens.elems[17].value.op == OP_XOR);
    assert(tokens.elems[18].value.op == OP_GT);
    assert(tokens.elems[19].value.op == OP_LT);
    assert(tokens.elems[20].value.op == OP_NOT);
    assert(tokens.elems[21].value.op == OP_ASSIGN);
    assert(tokens.elems[22].value.op == OP_QST);
    tokens_free(&tokens);
}

void test_tokenizer_idents() {
    // Identifiers
    char* src = "int x = 5; \n abc \n a \n _a \n 1a \n _ \na";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_KEYWORD);
    assert(tokens.elems[1].type == TK_IDENT);
    assert(tokens.elems[2].type == TK_OP);
    assert(tokens.elems[3].type == TK_LINT);
    assert(tokens.elems[4].type == TK_DELIMITER);
    assert(tokens.elems[5].type == TK_IDENT);
    assert(tokens.elems[6].type == TK_IDENT);
    assert(tokens.elems[7].type == TK_IDENT);
    assert(strcmp(tokens.elems[7].value.string, "_a") == 0);
    Token test = tokens.elems[8];
    assert(tokens.elems[8].type == TK_IDENT);
    assert(strcmp(tokens.elems[8].value.string, "_") == 0);
    assert(tokens.elems[9].type == TK_IDENT);
    tokens_free(&tokens);
}

void test_tokenizer_delims() {
    char* src = "{}()[],.;:";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_DELIMITER);
    assert(tokens.elems[0].value.delim == DL_OPENBRACE);
    assert(tokens.elems[1].value.delim == DL_CLOSEBRACE);
    assert(tokens.elems[2].value.delim == DL_OPENPAREN);
    assert(tokens.elems[3].value.delim == DL_CLOSEPAREN);
    assert(tokens.elems[4].value.delim == DL_OPENBRACKET);
    assert(tokens.elems[5].value.delim == DL_CLOSEBRACKET);
    assert(tokens.elems[6].value.delim == DL_COMMA);
    assert(tokens.elems[7].value.delim == DL_DOT);
    assert(tokens.elems[8].value.delim == DL_SEMICOLON);
    assert(tokens.elems[9].value.delim == DL_COLON);
    tokens_free(&tokens);
}

void test_tokenizer_values() {
    char* src = "13; \n3134\n 53asd; 1.3 .3 3. 1.3b ;";
    Tokens tokens = tokenize(src);
    // Ints
    assert(tokens.elems[0].type == TK_LINT);
    assert(strcmp(tokens.elems[0].value.string, "13") == 0);
    assert(tokens.elems[1].type == TK_DELIMITER);
    assert(tokens.elems[2].type == TK_LINT);
    assert(strcmp(tokens.elems[2].value.string, "3134") == 0);
    assert(tokens.elems[3].type == TK_DELIMITER);
    // Floats
    Token test = tokens.elems[4];
    assert(tokens.elems[4].type == TK_LFLOAT);
    assert(tokens.elems[5].type == TK_LFLOAT);
    assert(tokens.elems[6].type == TK_LFLOAT);
    assert(tokens.elems[7].type == TK_DELIMITER);
    tokens_free(&tokens);
}