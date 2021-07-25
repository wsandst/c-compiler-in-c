#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/tokens.h"
#include "../src/file_helpers.h"

// Declarations
void test_tokenizer();
void test_tokenizer_preprocessor();
void test_tokenizer_comments();
void test_tokenizer_strings();
void test_tokenizer_keywords();
void test_tokenizer_ops();
void test_tokenizer_idents();
void test_tokenizer_values();
void test_tokenizer_delims();
void test_tokenizer_large_src();

// Definitions
void test_tokenizer() {
    printf("[CTEST] Running tokenizer tests...\n");

    test_tokenizer_preprocessor();
    test_tokenizer_comments();
    test_tokenizer_strings();
    test_tokenizer_keywords();
    test_tokenizer_ops();
    test_tokenizer_idents();
    test_tokenizer_values();
    test_tokenizer_delims();

    test_tokenizer_large_src();

    printf("[CTEST] Passed tokenizer tests!\n");
}

void test_tokenizer_preprocessor() {
    // Preproccessor
    char* src = "#define\n   #include test\n";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_PREPROCESSOR);
    assert(tokens.elems[1].type == TK_PREPROCESSOR);
    assert(tokens.elems[2].type == TK_EOF); // Last token should always be EOF
    assert(strcmp(tokens.elems[1].value.string, "#include test") == 0);
    assert(tokens.size == 3);
    tokens_free(&tokens);

}

void test_tokenizer_comments() {
    // Comments
    char* src = "//#define\n #define \n while // hello \n/*test */ \n while /* \n if \n */while\n"
    "/*\n*/";
    Tokens tokens = tokenize(src);
    tokens_print(&tokens);
    assert(tokens.elems[0].type == TK_COMMENT);
    assert(strcmp(tokens.elems[1].value.string, "#define") == 0);
    assert(tokens.elems[1].type == TK_PREPROCESSOR);
    assert(tokens.elems[2].type == TK_KW_WHILE);
    assert(tokens.elems[3].type == TK_COMMENT);
    assert(tokens.elems[4].type == TK_COMMENT);
    assert(tokens.elems[5].type == TK_KW_WHILE);
    assert(tokens.elems[6].type == TK_COMMENT);
    assert(tokens.elems[7].type == TK_KW_WHILE);
    assert(tokens.elems[8].type == TK_COMMENT);
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
    assert(tokens.elems[0].type == TK_KW_UNSIGNED);
    assert(tokens.elems[1].type == TK_DL_DOT);
    assert(tokens.elems[2].type == TK_KW_IF);
    assert(tokens.elems[3].type == TK_KW_ELSE);
    assert(tokens.elems[4].type == TK_KW_WHILE);
    assert(tokens.elems[5].type == TK_KW_DO);
    assert(tokens.elems[6].type == TK_KW_FOR);
    assert(tokens.elems[7].type == TK_KW_BREAK);
    assert(tokens.elems[8].type == TK_KW_CONTINUE);
    assert(tokens.elems[9].type == TK_KW_RETURN);
    assert(tokens.elems[10].type == TK_KW_SWITCH);
    assert(tokens.elems[11].type == TK_KW_CASE);
    assert(tokens.elems[12].type == TK_KW_DEFAULT);
    assert(tokens.elems[13].type == TK_KW_GOTO);
    assert(tokens.elems[14].type == TK_KW_LABEL);
    assert(tokens.elems[15].type == TK_KW_TYPEDEF);
    assert(tokens.elems[16].type == TK_KW_STRUCT);
    assert(tokens.elems[17].type == TK_KW_UNION);
    assert(tokens.elems[18].type == TK_KW_CONST);
    assert(tokens.elems[19].type == TK_KW_LONG);
    assert(tokens.elems[20].type == TK_KW_SHORT);
    assert(tokens.elems[21].type == TK_KW_SIGNED);
    assert(tokens.elems[22].type == TK_KW_INT);
    assert(tokens.elems[23].type == TK_KW_FLOAT);
    assert(tokens.elems[24].type == TK_KW_DOUBLE);
    assert(tokens.elems[25].type == TK_KW_CHAR);
    assert(tokens.elems[26].type == TK_KW_VOID);
    tokens_free(&tokens);
}

void test_tokenizer_ops() {
    // Operations
    char* src = "|| && >> << == != ** >= <= + - * / % | & ~ ^ > < ! = ?";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type  == TK_OP_OR);
    assert(tokens.elems[1].type  == TK_OP_AND);
    assert(tokens.elems[2].type  == TK_OP_RIGHTSHIFT);
    assert(tokens.elems[3].type  == TK_OP_LEFTSHIFT);
    assert(tokens.elems[4].type  == TK_OP_EQ);
    assert(tokens.elems[5].type  == TK_OP_NEQ);
    assert(tokens.elems[6].type  == TK_OP_EXP);
    assert(tokens.elems[7].type  == TK_OP_GTE);
    assert(tokens.elems[8].type  == TK_OP_LTE);
    assert(tokens.elems[9].type  == TK_OP_PLUS);
    assert(tokens.elems[10].type == TK_OP_MINUS);
    assert(tokens.elems[11].type == TK_OP_MULT);
    assert(tokens.elems[12].type == TK_OP_DIV);
    assert(tokens.elems[13].type == TK_OP_MOD);
    assert(tokens.elems[14].type == TK_OP_BITOR);
    assert(tokens.elems[15].type == TK_OP_BITAND);
    assert(tokens.elems[16].type == TK_OP_COMPL);
    assert(tokens.elems[17].type == TK_OP_XOR);
    assert(tokens.elems[18].type == TK_OP_GT);
    assert(tokens.elems[19].type == TK_OP_LT);
    assert(tokens.elems[20].type == TK_OP_NOT);
    assert(tokens.elems[21].type == TK_OP_ASSIGN);
    assert(tokens.elems[22].type == TK_OP_QST);
    tokens_free(&tokens);
}

void test_tokenizer_idents() {
    // Identifiers
    char* src = "int x = 5; \n abc \n a \n _a \n 1a \n _ \na";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_KW_INT);
    assert(tokens.elems[1].type == TK_IDENT);
    assert(tokens.elems[2].type == TK_OP_ASSIGN);
    assert(tokens.elems[3].type == TK_LINT);
    assert(tokens.elems[4].type == TK_DL_SEMICOLON);
    assert(tokens.elems[5].type == TK_IDENT);
    assert(tokens.elems[6].type == TK_IDENT);
    assert(tokens.elems[7].type == TK_IDENT);
    assert(strcmp(tokens.elems[7].value.string, "_a") == 0);
    assert(tokens.elems[8].type == TK_IDENT);
    assert(strcmp(tokens.elems[8].value.string, "_") == 0);
    assert(tokens.elems[9].type == TK_IDENT);
    tokens_free(&tokens);
}

void test_tokenizer_delims() {
    char* src = "{}()[],.;:";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_DL_OPENBRACE);
    assert(tokens.elems[1].type == TK_DL_CLOSEBRACE);
    assert(tokens.elems[2].type == TK_DL_OPENPAREN);
    assert(tokens.elems[3].type == TK_DL_CLOSEPAREN);
    assert(tokens.elems[4].type == TK_DL_OPENBRACKET);
    assert(tokens.elems[5].type == TK_DL_CLOSEBRACKET);
    assert(tokens.elems[6].type == TK_DL_COMMA);
    assert(tokens.elems[7].type == TK_DL_DOT);
    assert(tokens.elems[8].type == TK_DL_SEMICOLON);
    assert(tokens.elems[9].type == TK_DL_COLON);
    tokens_free(&tokens);
}

void test_tokenizer_values() {
    char* src = "13; \n3134\n 53asd; 1.3 .3 3. 1.3b ;";
    Tokens tokens = tokenize(src);
    // Ints
    assert(tokens.elems[0].type == TK_LINT);
    assert(strcmp(tokens.elems[0].string_repr, "13") == 0);
    assert(tokens.elems[1].type == TK_DL_SEMICOLON);
    assert(tokens.elems[2].type == TK_LINT);
    assert(strcmp(tokens.elems[2].string_repr, "3134") == 0);
    assert(tokens.elems[3].type == TK_DL_SEMICOLON);
    // Floats
    assert(tokens.elems[4].type == TK_LFLOAT);
    assert(tokens.elems[5].type == TK_LFLOAT);
    assert(tokens.elems[6].type == TK_LFLOAT);
    assert(tokens.elems[7].type == TK_DL_DOT);
    assert(tokens.elems[8].type == TK_DL_SEMICOLON);
    tokens_free(&tokens);
}

void test_tokenizer_large_src() {
    char* src = load_file_to_string("test/test_code/tokenizer_test.c");

    // Manually tokenize
    // We need access to the overwritten StrVector lines to ensure every token was grabbed
    int src_length = strlen(src);
    Tokens tokens = tokens_new(src_length);

    StrVector lines = str_split(src, '\n');
    for (size_t i = 0; i < lines.size; i++)
    {
        char* new_str = str_strip(lines.elems[i]);
        free(lines.elems[i]);
        lines.elems[i] = new_str;
    }
    
    tokenize_preprocessor(&tokens, &lines);
    tokenize_comments(&tokens, &lines);
    tokenize_strings(&tokens, &lines);
    tokenize_keywords(&tokens, &lines);
    tokenize_ops(&tokens, &lines);
    tokenize_idents(&tokens, &lines);
    tokenize_values(&tokens, &lines);
    tokenize_delims(&tokens, &lines);

    // Make sure every token was grabbed, only whitespace should be left
    for (size_t i = 0; i < lines.size; i++)
    {
        char* str = lines.elems[i];
        while (*str != '\0') {
            if (*str != ' ' && *str != '\t' && *str != '\n') {
                assert(false);
            }
            str++;
        }
    }

    tokens_trim(&tokens);

    // Check some tokens manually to make sure everything works as intended
    assert(tokens.elems[0].type == TK_PREPROCESSOR);
    assert(tokens.elems[2].type == TK_IDENT);
    assert(tokens.elems[tokens.size-1].type == TK_DL_CLOSEBRACE);
    assert(tokens.elems[tokens.size-6].type == TK_DL_SEMICOLON);
    assert(tokens.elems[tokens.size-7].type == TK_IDENT);
    assert(tokens.elems[tokens.size-8].type == TK_OP_PLUS);

    tokens_free(&tokens);
    str_vec_free(&lines);
    free(src);
}