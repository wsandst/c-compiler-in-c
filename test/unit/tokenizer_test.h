#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/tokens.h"
#include "../src/util/file_helpers.h"

// Declarations
void test_tokenizer();
void test_tokenizer_helpers();
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

    test_tokenizer_helpers();
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

void test_tokenizer_helpers() {
    // String conversion
    assert(strcmp(token_type_to_string(TK_OP_ASSIGN_BITOR), "TK_OP_ASSIGN_BITOR") == 0);
    assert(strcmp(token_type_to_string(TK_KW_VOID), "TK_KW_VOID") == 0);
    assert(strcmp(token_type_to_string(TK_LINT), "TK_LINT") == 0);

    // Token insertion/concatenation
    Tokens tokens1 = tokens_new(4);
    //tokens_get(&combined_tokens, 
    //tokens_get(&tokens, 
    tokens_get(&tokens1, 0)->type = 1;
    tokens_get(&tokens1, 1)->type = 2;
    tokens_get(&tokens1, 2)->type = 6;
    tokens_get(&tokens1, 3)->type = 7;
    Tokens tokens2 = tokens_new(3);
    tokens_get(&tokens2, 0)->type = 3;
    tokens_get(&tokens2, 1)->type = 4;
    tokens_get(&tokens2, 2)->type = 5;
    Tokens* combined_tokens = tokens_insert(&tokens1, &tokens2, 2);
    assert(combined_tokens->size == 7);
    assert(tokens_get(combined_tokens, 0)->type == 1);
    assert(tokens_get(combined_tokens, 1)->type == 2);
    assert(tokens_get(combined_tokens, 2)->type == 3);
    assert(tokens_get(combined_tokens, 3)->type == 4);
    assert(tokens_get(combined_tokens, 4)->type == 5);
    assert(tokens_get(combined_tokens, 5)->type == 6);
    assert(tokens_get(combined_tokens, 6)->type == 7);
    tokens_free(&tokens1);
    tokens_free(&tokens2);
}

void test_tokenizer_preprocessor() {
    // Preproccessor
    char* src = "#define\n   #include test\n";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type == TK_PREPROCESSOR);
    assert(tokens_get(&tokens, 1)->type == TK_PREPROCESSOR);
    assert(tokens_get(&tokens, 2)->type == TK_EOF); // Last token should always be EOF
    assert(strcmp(tokens_get(&tokens, 1)->string_repr, "#include test") == 0);
    assert(tokens.size == 3);
    tokens_free(&tokens);
}

void test_tokenizer_comments() {
    // Comments
    char* src = "//#define\n #define \n while // hello \n/*test */ \n while /* \n if \n */while\n"
    "/*\n*/";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type == TK_COMMENT);
    assert(strcmp(tokens_get(&tokens, 1)->string_repr, "#define") == 0);
    assert(tokens_get(&tokens, 1)->type == TK_PREPROCESSOR);
    assert(tokens_get(&tokens, 2)->type == TK_KW_WHILE);
    assert(tokens_get(&tokens, 3)->type == TK_COMMENT);
    assert(tokens_get(&tokens, 4)->type == TK_COMMENT);
    assert(tokens_get(&tokens, 5)->type == TK_KW_WHILE);
    assert(tokens_get(&tokens, 6)->type == TK_COMMENT);
    assert(tokens_get(&tokens, 7)->type == TK_KW_WHILE);
    assert(tokens_get(&tokens, 8)->type == TK_COMMENT);
    tokens_free(&tokens);
}

void test_tokenizer_strings() {
    // Strings
    char* src = "//\"\"\n \"hello\" \n \"hello\\\"\" \n 'c' \n '\\n' '\\\"'";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type == TK_COMMENT);
    assert(tokens_get(&tokens, 1)->type == TK_LSTRING);
    assert(strcmp(tokens_get(&tokens, 1)->string_repr, "hello") == 0);
    assert(tokens_get(&tokens, 2)->type == TK_LSTRING);
    assert(strcmp(tokens_get(&tokens, 2)->string_repr, "hello\\\"") == 0);
    assert(tokens_get(&tokens, 3)->type == TK_LCHAR);
    assert(strcmp(tokens_get(&tokens, 3)->string_repr, "c") == 0);
    assert(tokens_get(&tokens, 4)->type == TK_LCHAR);
    assert(strcmp(tokens_get(&tokens, 4)->string_repr, "\\n") == 0);
    assert(tokens_get(&tokens, 5)->type == TK_LCHAR);
    assert(strcmp(tokens_get(&tokens, 5)->string_repr, "\\\"") == 0);
    tokens_free(&tokens);
}

void test_tokenizer_keywords() {
    // Keywords
    char* src = "unsigned.if else while do for break continue return switch case\ndefault " 
          "goto label typedef struct union const long short signed "
          "int float double char void hello_int int_hello _int";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type == TK_KW_UNSIGNED);
    assert(tokens_get(&tokens, 1)->type == TK_DL_DOT);
    assert(tokens_get(&tokens, 2)->type == TK_KW_IF);
    assert(tokens_get(&tokens, 3)->type == TK_KW_ELSE);
    assert(tokens_get(&tokens, 4)->type == TK_KW_WHILE);
    assert(tokens_get(&tokens, 5)->type == TK_KW_DO);
    assert(tokens_get(&tokens, 6)->type == TK_KW_FOR);
    assert(tokens_get(&tokens, 7)->type == TK_KW_BREAK);
    assert(tokens_get(&tokens, 8)->type == TK_KW_CONTINUE);
    assert(tokens_get(&tokens, 9)->type == TK_KW_RETURN);
    assert(tokens_get(&tokens, 10)->type == TK_KW_SWITCH);
    assert(tokens_get(&tokens, 11)->type == TK_KW_CASE);
    assert(tokens_get(&tokens, 12)->type == TK_KW_DEFAULT);
    assert(tokens_get(&tokens, 13)->type == TK_KW_GOTO);
    assert(tokens_get(&tokens, 14)->type == TK_KW_LABEL);
    assert(tokens_get(&tokens, 15)->type == TK_KW_TYPEDEF);
    assert(tokens_get(&tokens, 16)->type == TK_KW_STRUCT);
    assert(tokens_get(&tokens, 17)->type == TK_KW_UNION);
    assert(tokens_get(&tokens, 18)->type == TK_KW_CONST);
    assert(tokens_get(&tokens, 19)->type == TK_KW_LONG);
    assert(tokens_get(&tokens, 20)->type == TK_KW_SHORT);
    assert(tokens_get(&tokens, 21)->type == TK_KW_SIGNED);
    assert(tokens_get(&tokens, 22)->type == TK_KW_INT);
    assert(tokens_get(&tokens, 23)->type == TK_KW_FLOAT);
    assert(tokens_get(&tokens, 24)->type == TK_KW_DOUBLE);
    assert(tokens_get(&tokens, 25)->type == TK_KW_CHAR);
    assert(tokens_get(&tokens, 26)->type == TK_KW_VOID);
    assert(tokens_get(&tokens, 27)->type == TK_IDENT);
    assert(tokens_get(&tokens, 28)->type == TK_IDENT);
    assert(tokens_get(&tokens, 29)->type == TK_IDENT);
    assert(tokens_get(&tokens, 30)->type == TK_EOF);
    tokens_free(&tokens);
}

void test_tokenizer_ops() {
    // Operations
    char* src = "|| && >> << == != >= <= + - * / % | & ~ ^ > < ! = ? ++ -- += -= *= /= %= <<= >>= &= |= ^= sizeof";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type  == TK_OP_OR);
    assert(tokens_get(&tokens, 1)->type  == TK_OP_AND);
    assert(tokens_get(&tokens, 2)->type  == TK_OP_RIGHTSHIFT);
    assert(tokens_get(&tokens, 3)->type  == TK_OP_LEFTSHIFT);
    assert(tokens_get(&tokens, 4)->type  == TK_OP_EQ);
    assert(tokens_get(&tokens, 5)->type  == TK_OP_NEQ);
    assert(tokens_get(&tokens, 6)->type  == TK_OP_GTE);
    assert(tokens_get(&tokens, 7)->type  == TK_OP_LTE);
    assert(tokens_get(&tokens, 8)->type  == TK_OP_PLUS);
    assert(tokens_get(&tokens, 9)->type  == TK_OP_MINUS);
    assert(tokens_get(&tokens, 10)->type == TK_OP_MULT);
    assert(tokens_get(&tokens, 11)->type == TK_OP_DIV);
    assert(tokens_get(&tokens, 12)->type == TK_OP_MOD);
    assert(tokens_get(&tokens, 13)->type == TK_OP_BITOR);
    assert(tokens_get(&tokens, 14)->type == TK_OP_BITAND);
    assert(tokens_get(&tokens, 15)->type == TK_OP_COMPL);
    assert(tokens_get(&tokens, 16)->type == TK_OP_BITXOR);
    assert(tokens_get(&tokens, 17)->type == TK_OP_GT);
    assert(tokens_get(&tokens, 18)->type == TK_OP_LT);
    assert(tokens_get(&tokens, 19)->type == TK_OP_NOT);
    assert(tokens_get(&tokens, 20)->type == TK_OP_ASSIGN);
    assert(tokens_get(&tokens, 21)->type == TK_OP_QST);
    assert(tokens_get(&tokens, 22)->type == TK_OP_INCR);
    assert(tokens_get(&tokens, 23)->type == TK_OP_DECR);
    assert(tokens_get(&tokens, 24)->type == TK_OP_ASSIGN_ADD);
    assert(tokens_get(&tokens, 25)->type == TK_OP_ASSIGN_SUB);
    assert(tokens_get(&tokens, 26)->type == TK_OP_ASSIGN_MULT);
    assert(tokens_get(&tokens, 27)->type == TK_OP_ASSIGN_DIV);
    assert(tokens_get(&tokens, 28)->type == TK_OP_ASSIGN_MOD);
    assert(tokens_get(&tokens, 29)->type == TK_OP_ASSIGN_LEFTSHIFT);
    assert(tokens_get(&tokens, 30)->type == TK_OP_ASSIGN_RIGHTSHIFT);
    assert(tokens_get(&tokens, 31)->type == TK_OP_ASSIGN_BITAND);
    assert(tokens_get(&tokens, 32)->type == TK_OP_ASSIGN_BITOR);
    assert(tokens_get(&tokens, 33)->type == TK_OP_ASSIGN_BITXOR);
    assert(tokens_get(&tokens, 34)->type == TK_OP_SIZEOF);

    tokens_free(&tokens);
}

void test_tokenizer_idents() {
    // Identifiers
    char* src = "int x = 5; \n abc \n a \n _a \n 1a \n _ \na\nabc_efg";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type == TK_KW_INT);
    assert(tokens_get(&tokens, 1)->type == TK_IDENT);
    assert(tokens_get(&tokens, 2)->type == TK_OP_ASSIGN);
    assert(tokens_get(&tokens, 3)->type == TK_LINT);
    assert(tokens_get(&tokens, 4)->type == TK_DL_SEMICOLON);
    assert(tokens_get(&tokens, 5)->type == TK_IDENT);
    assert(tokens_get(&tokens, 6)->type == TK_IDENT);
    assert(tokens_get(&tokens, 7)->type == TK_IDENT);
    assert(strcmp(tokens_get(&tokens, 7)->string_repr, "_a") == 0);
    assert(tokens_get(&tokens, 8)->type == TK_IDENT);
    assert(strcmp(tokens_get(&tokens, 8)->string_repr, "_") == 0);
    assert(tokens_get(&tokens, 9)->type == TK_IDENT);
    assert(strcmp(tokens_get(&tokens, 10)->string_repr, "abc_efg") == 0);
    assert(tokens_get(&tokens, 10)->type == TK_IDENT);
    tokens_free(&tokens);
}

void test_tokenizer_delims() {
    char* src = "{}()[],.;:";
    Tokens tokens = tokenize(src);
    assert(tokens_get(&tokens, 0)->type == TK_DL_OPENBRACE);
    assert(tokens_get(&tokens, 1)->type == TK_DL_CLOSEBRACE);
    assert(tokens_get(&tokens, 2)->type == TK_DL_OPENPAREN);
    assert(tokens_get(&tokens, 3)->type == TK_DL_CLOSEPAREN);
    assert(tokens_get(&tokens, 4)->type == TK_DL_OPENBRACKET);
    assert(tokens_get(&tokens, 5)->type == TK_DL_CLOSEBRACKET);
    assert(tokens_get(&tokens, 6)->type == TK_DL_COMMA);
    assert(tokens_get(&tokens, 7)->type == TK_DL_DOT);
    assert(tokens_get(&tokens, 8)->type == TK_DL_SEMICOLON);
    assert(tokens_get(&tokens, 9)->type == TK_DL_COLON);
    tokens_free(&tokens);
}

void test_tokenizer_values() {
    char* src = "13; \n3134\n 53asd; 1.3 .3 3. 1.3b ;";
    Tokens tokens = tokenize(src);
    // Ints
    assert(tokens_get(&tokens, 0)->type == TK_LINT);
    assert(strcmp(tokens_get(&tokens, 0)->string_repr, "13") == 0);
    assert(tokens_get(&tokens, 1)->type == TK_DL_SEMICOLON);
    assert(tokens_get(&tokens, 2)->type == TK_LINT);
    assert(strcmp(tokens_get(&tokens, 2)->string_repr, "3134") == 0);
    assert(tokens_get(&tokens, 3)->type == TK_DL_SEMICOLON);
    // Floats
    assert(tokens_get(&tokens, 4)->type == TK_LFLOAT);
    assert(tokens_get(&tokens, 5)->type == TK_LFLOAT);
    assert(tokens_get(&tokens, 6)->type == TK_LFLOAT);
    assert(tokens_get(&tokens, 7)->type == TK_DL_DOT);
    assert(tokens_get(&tokens, 8)->type == TK_DL_SEMICOLON);
    tokens_free(&tokens);
}

void test_tokenizer_large_src() {
    char* src = load_file_to_string("test/unit/examples/example_code.c");

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
    assert(tokens_get(&tokens, 0)->type == TK_PREPROCESSOR);
    assert(tokens_get(&tokens, 2)->type  == TK_IDENT);
    assert(tokens_get(&tokens, tokens.size-1)->type == TK_DL_CLOSEBRACE);
    assert(tokens_get(&tokens, tokens.size-6)->type == TK_DL_SEMICOLON);
    assert(tokens_get(&tokens, tokens.size-7)->type == TK_IDENT);
    assert(tokens_get(&tokens, tokens.size-8)->type == TK_OP_PLUS);

    tokens_free(&tokens);
    str_vec_free(&lines);
    free(src);
}