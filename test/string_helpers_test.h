#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/string_helpers.h"

void test_string_helpers() {
    printf("[TEST] Running string helper tests...\n");
    test_str_vec();
    test_helper_funcs();
    printf("[TEST] Passed string helper tests!\n");
}

void test_str_vec() {
    StrVector vec = str_split(" hello world  test ", ' ');
    assert(vec.size == 3);
    assert(strcmp(vec.elems[0], "hello") == 0);
    assert(strcmp(vec.elems[1], "world") == 0);
    assert(strcmp(vec.elems[2], "test") == 0);
    str_vec_free(&vec);
}

void test_helper_funcs() {
    char* test_str = "hello world";
    // str_copy()
    char* copy_str = str_copy(test_str);
    assert(strcmp(test_str, copy_str) == 0);
    free(copy_str);

    // str_substr()
    char* substr = str_substr(test_str, 5);
    assert(strcmp(substr, "hello") == 0);
    free(substr);

    // str_startswith()
    assert(str_startswith(test_str, "he"));
    assert(str_startswith(test_str, "hello"));
    assert(!str_startswith(test_str, "hey"));
    assert(str_startswith(test_str, ""));
    // Intended edge case. We match if the match string is longer than the actual string
    assert(str_startswith(test_str, "hello world "));

    // str_endswith()
    assert(str_endswith(test_str, "rld"));
    assert(str_endswith(test_str, "world"));
    assert(!str_endswith(test_str, "hey"));
    // Edge case, works differently than startswith()
    assert(!str_endswith(test_str, ""));
    // Intended edge case. We match if the match string is longer than the actual string
    assert(str_endswith(test_str, " hello world"));

    // str_contains()
    assert(str_contains(test_str, "hello") == 1);
    assert(str_contains(test_str, "hello world") == 1);
    assert(str_contains(test_str, "o wo") == 5);
    assert(str_contains(test_str, "no") == 0);
    assert(str_contains(test_str, "hello world cool") == 0);
    assert(str_contains(test_str, " hello") == 0);

    // str_contains_word()
    test_str = "word1: word2\n word3( word4 word5not notword5 word6";
    assert(str_contains_word(test_str, "word1"));
    assert(str_contains_word(test_str, "word2"));
    assert(str_contains_word(test_str, "word3"));
    assert(str_contains_word(test_str, "word4"));
    assert(str_contains_word(test_str, "word6"));
    assert(!str_contains_word(test_str, "no"));
    assert(!str_contains_word(test_str, "word5"));

    // str_strip()
    test_str = "   \t\n hello world   \t\n";

    test_str = str_strip(test_str);
    char* result_str = "hello world";

    assert(strcmp(test_str, result_str) == 0);
    free(test_str);

    test_str = str_strip(" ");
    free(test_str);

    // str_fill()
    test_str = malloc(10*sizeof(char));
    str_fill(test_str, strlen(test_str), 't');
    for (size_t i = 0; i < strlen(test_str); i++) {
        assert(test_str[i] == 't');
    }
    
    free(test_str);
}