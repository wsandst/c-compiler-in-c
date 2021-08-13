#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/util/string_helpers.h"

void test_string_helpers();
void test_str_vec();
void test_string_helper_funcs();

void test_string_helpers() {
    printf("[CTEST] Running string helper tests...\n");
    test_str_vec();
    test_string_helper_funcs();
    printf("[CTEST] Passed string helper tests!\n");
}

void test_str_vec() {
    StrVector vec = str_split(" hello world  test ", ' ');
    assert(vec.size == 3);
    assert(strcmp(vec.elems[0], "hello") == 0);
    assert(strcmp(vec.elems[1], "world") == 0);
    assert(strcmp(vec.elems[2], "test") == 0);
 
    str_vec_push(&vec, "test2");
    assert(strcmp(vec.elems[3], "test2") == 0);

    char *joined_str = str_vec_join(&vec);
    assert(strcmp(joined_str, "helloworldtesttest2") == 0);
    free(joined_str);

    joined_str = str_vec_join_with_delim(&vec, ' ');
    assert(strcmp(joined_str, "hello world test test2 ") == 0);
    free(joined_str);

    // Slice
    StrVector slice_vec = str_vec_slice(&vec, 1, 3);
    joined_str = str_vec_join(&slice_vec);
    assert(strcmp(joined_str, "worldtest") == 0);
    free(joined_str);

    // Add two StrVectors
    StrVector vec1 = str_split("hello world cool", ' ');
    StrVector vec2 = str_split("more cool stuff", ' ');

    StrVector* added_vec = str_vec_add(&vec1, &vec2);
    assert(added_vec == &vec1);
    assert(added_vec->size == 6);
    assert(added_vec->max_size == 6);
    assert(strcmp(added_vec->elems[2], "cool") == 0);
    assert(strcmp(added_vec->elems[3], "more") == 0);
    
    str_vec_free(added_vec);
    str_vec_free(&vec);
}

void test_string_helper_funcs() {
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

    test_str = "hello/world/test";
    int index = str_index_of_reverse(test_str, '/');
    assert(test_str[index] == '/');
    test_str = "hesdawlo/wodwadrld/";
    index = str_index_of_reverse(test_str, '/');
    assert(test_str[index] == '/');

    // str_strip()
    test_str = "   \t\n hello world   \t\n";

    test_str = str_strip(test_str);
    char* result_str = "hello world";

    assert(strcmp(test_str, result_str) == 0);
    free(test_str);

    test_str = str_strip(" ");
    free(test_str);

    // str_fill()
    test_str = calloc(10, sizeof(char));
    str_fill(test_str, strlen(test_str), 't');
    for (size_t i = 0; i < strlen(test_str); i++) {
        assert(test_str[i] == 't');
    }

    // Str add
    char *added_str = str_add("hello ", "world");
    assert(strcmp(added_str, "hello world") == 0);
    free(added_str);

    // Str multiply
    char *multiplied_str = str_multiply("a", 5);
    assert(strcmp(multiplied_str, "aaaaa") == 0);
    free(multiplied_str);

    multiplied_str = str_multiply("a", 0);
    assert(strcmp(multiplied_str, "") == 0);
    free(multiplied_str);
    
    free(test_str);
}