#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/tokens.h"

void test_tokenizer() {
    printf("[TEST] Running tokenizer tests...\n");

    // Preproccessor
    char* src = "#define\n   #include test\n";
    Tokens tokens = tokenize(src);
    assert(tokens.elems[0].type == TK_PREPROCESSOR);
    assert(tokens.elems[1].type == TK_PREPROCESSOR);
    assert(strcmp(tokens.elems[1].data.string, "#include test") == 0);
    assert(tokens.size == 2);
    tokens_free(&tokens);

    // Comments
    src = "//#define\n #define \n while // hello \n /* test */ \n while /* \n if \n */while";
    tokens = tokenize(src);
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

    // Strings
    src = "//\"\"\n \"hello\" \n \"hello\\\"\" \n 'c' \n '\\n' '\\\"'";
    tokens = tokenize(src);
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

    // Keywords
    src = "unsigned if else:while/do;for(break{continue.return,switch[case\ndefault " 
          "goto label typedef struct union const long short signed";
    tokens = tokenize(src);
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

    printf("[TEST] Passed tokenizer tests!\n");
}