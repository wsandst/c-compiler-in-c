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

void test_tokenizer() {
    printf("[TEST] Running tokenizer tests...\n");

    //test_tokenizer_preprocessor();
    //test_tokenizer_comments();
    //test_tokenizer_strings();
    //test_tokenizer_keywords();
    //test_tokenizer_ops();
    test_tokenizer_idents();

    printf("[TEST] Passed tokenizer tests!\n");
}

void test_tokenizer_preprocessor() {
    // Preproccessor
    char* src = "#define\n   #include test\n";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_PREPROCESSOR);
    assert(tokens.elems[1].type == TK_PREPROCESSOR);
    assert(strcmp(tokens.elems[1].data.string, "#include test") == 0);
    assert(tokens.size == 2);
    tokens_free(&tokens);

}

void test_tokenizer_comments() {
    // Comments
    char* src = "//#define\n #define \n while // hello \n /* test */ \n while /* \n if \n */while";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_COMMENT);
    assert(strcmp(tokens.elems[1].data.string, "#define") == 0);
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
    assert(tokens.elems[1].type == TK_LITERAL);
    assert(tokens.elems[1].sub_type.literal == TK_LSTRING);
    assert(strcmp(tokens.elems[1].data.string, "hello") == 0);
    assert(tokens.elems[2].sub_type.literal == TK_LSTRING);
    assert(strcmp(tokens.elems[2].data.string, "hello\\\"") == 0);
    assert(tokens.elems[3].sub_type.literal == TK_LCHAR);
    assert(strcmp(tokens.elems[3].data.string, "c") == 0);
    assert(tokens.elems[4].sub_type.literal == TK_LCHAR);
    assert(strcmp(tokens.elems[4].data.string, "\\n") == 0);
    assert(tokens.elems[5].sub_type.literal == TK_LCHAR);
    assert(strcmp(tokens.elems[5].data.string, "\\\"") == 0);
    tokens_free(&tokens);
}

void test_tokenizer_keywords() {
    // Keywords
    char* src = "unsigned if else:while do;for(break{continue.return,switch[case\ndefault " 
          "goto label typedef struct union const long short signed "
          "int float double char void";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].sub_type.keyword == TK_UNSIGNED);
    assert(tokens.elems[1].sub_type.keyword == TK_IF);
    assert(tokens.elems[2].sub_type.keyword == TK_ELSE);
    assert(tokens.elems[3].sub_type.keyword == TK_WHILE);
    assert(tokens.elems[4].sub_type.keyword == TK_DO);
    assert(tokens.elems[5].sub_type.keyword == TK_FOR);
    assert(tokens.elems[6].sub_type.keyword == TK_BREAK);
    assert(tokens.elems[7].sub_type.keyword == TK_CONTINUE);
    assert(tokens.elems[8].sub_type.keyword == TK_RETURN);
    assert(tokens.elems[9].sub_type.keyword == TK_SWITCH);
    assert(tokens.elems[10].sub_type.keyword == TK_CASE);
    assert(tokens.elems[11].sub_type.keyword == TK_DEFAULT);
    assert(tokens.elems[12].sub_type.keyword == TK_GOTO);
    assert(tokens.elems[13].sub_type.keyword == TK_LABEL);
    assert(tokens.elems[14].sub_type.keyword == TK_TYPEDEF);
    assert(tokens.elems[15].sub_type.keyword == TK_STRUCT);
    assert(tokens.elems[16].sub_type.keyword == TK_UNION);
    assert(tokens.elems[17].sub_type.keyword == TK_CONST);
    assert(tokens.elems[18].sub_type.keyword == TK_LONG);
    assert(tokens.elems[19].sub_type.keyword == TK_SHORT);
    assert(tokens.elems[20].sub_type.keyword == TK_SIGNED);
    assert(tokens.elems[21].sub_type.keyword == TK_INT);
    assert(tokens.elems[22].sub_type.keyword == TK_FLOAT);
    assert(tokens.elems[23].sub_type.keyword == TK_DOUBLE);
    assert(tokens.elems[24].sub_type.keyword == TK_CHAR);
    assert(tokens.elems[25].sub_type.keyword == TK_VOID);
    tokens_free(&tokens);
}

void test_tokenizer_ops() {
    // Operations
    char* src = "|| && >> << == != ** >= <= + - * / % | & ~ ^ > < ! = ?";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].sub_type.op == TK_OR);
    assert(tokens.elems[1].sub_type.op == TK_AND);
    assert(tokens.elems[2].sub_type.op == TK_RIGHTSHIFT);
    assert(tokens.elems[3].sub_type.op == TK_LEFTSHIFT);
    assert(tokens.elems[4].sub_type.op == TK_EQ);
    assert(tokens.elems[5].sub_type.op == TK_NEQ);
    assert(tokens.elems[6].sub_type.op == TK_EXP);
    assert(tokens.elems[7].sub_type.op == TK_GTE);
    assert(tokens.elems[8].sub_type.op == TK_LTE);
    assert(tokens.elems[9].sub_type.op == TK_PLUS);
    assert(tokens.elems[10].sub_type.op == TK_MINUS);
    assert(tokens.elems[11].sub_type.op == TK_MULT);
    assert(tokens.elems[12].sub_type.op == TK_DIV);
    assert(tokens.elems[13].sub_type.op == TK_MOD);
    assert(tokens.elems[14].sub_type.op == TK_BITOR);
    assert(tokens.elems[15].sub_type.op == TK_BITAND);
    assert(tokens.elems[16].sub_type.op == TK_COMPL);
    assert(tokens.elems[17].sub_type.op == TK_XOR);
    assert(tokens.elems[18].sub_type.op == TK_GT);
    assert(tokens.elems[19].sub_type.op == TK_LT);
    assert(tokens.elems[20].sub_type.op == TK_NOT);
    assert(tokens.elems[21].sub_type.op == TK_ASSIGN);
    assert(tokens.elems[22].sub_type.op == TK_QST);
    tokens_free(&tokens);
}

void test_tokenizer_idents() {
    // Identifiers
    char* src = "int x = 5; \n abc \n a \n _a \n 1a \n _ \na";
    Tokens tokens = tokenize(src);
    tokens_print(&tokens);
    assert(tokens.elems[0].type == TK_KEYWORD);
    assert(tokens.elems[1].type == TK_IDENT);
    assert(tokens.elems[2].type == TK_OP);
    assert(tokens.elems[3].type == TK_IDENT);
    assert(tokens.elems[4].type == TK_IDENT);
    assert(tokens.elems[5].type == TK_IDENT);
    assert(strcmp(tokens.elems[5].data.string, "_a") == 0);
    Token test = tokens.elems[6];
    assert(tokens.elems[6].type == TK_IDENT);
    assert(strcmp(tokens.elems[6].data.string, "_") == 0);
    assert(tokens.elems[7].type == TK_IDENT);
    tokens_free(&tokens);
}