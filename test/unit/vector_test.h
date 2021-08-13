#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/util/vector.h"

struct TestStruct {
    int x;
    char c;
};

typedef struct TestStruct TestStruct;

void test_vector();
void test_vec_insert();

void test_vector() {
    printf("[CTEST] Running vector tests...\n");

    int elem = 1;
    Vec vec = vec_new(sizeof(int), 1);
    vec_push(&vec, &elem);
    assert(vec.size == 1);
    elem = 2;
    vec_push(&vec, &elem);
    elem = 3;
    vec_push(&vec, &elem);
    assert(vec.size == 3);
    assert(vec.max_size == 4);

    int* res = (int*) vec_get(&vec, 0);
    assert(*res == 1);
    res = (int*) vec_get(&vec, 1);
    assert(*res == 2);
    res = (int*) vec_get(&vec, 2);
    assert(*res == 3);
    res = (int*) vec_peek(&vec);
    assert(*res == 3);
    res = (int*) vec_pop(&vec);
    assert(*res == 3);
    assert(vec.size == 2);

    vec_free(&vec);

    // Test with a struct
    TestStruct struct_elem;
    struct_elem.x = 1;
    struct_elem.c = 2;
    vec = vec_new(sizeof(TestStruct), 1);
    vec_push(&vec, &struct_elem);
    struct_elem.x = 3;
    struct_elem.c = 4;
    vec_push(&vec, &struct_elem);
    struct_elem.x = 5;
    struct_elem.c = 6;
    vec_push(&vec, &struct_elem);
    struct_elem = *((TestStruct*) vec_get(&vec, 0));
    assert(struct_elem.x == 1);
    assert(struct_elem.c == 2);
    struct_elem = *((TestStruct*) vec_get(&vec, 1));
    assert(struct_elem.x == 3);
    assert(struct_elem.c == 4);
    struct_elem = *((TestStruct*) vec_get(&vec, 2));
    assert(struct_elem.x == 5);
    assert(struct_elem.c == 6);
    vec_free(&vec);

    test_vec_insert();

    printf("[CTEST] Passed vector tests!\n");
}

void test_vec_insert() {
    Vec vec1 = vec_new(sizeof(int), 1);
    int elem = 1;
    vec_push(&vec1, &elem);
    elem = 2;
    vec_push(&vec1, &elem);
    elem = 6;
    vec_push(&vec1, &elem);
    elem = 7;
    vec_push(&vec1, &elem);

    Vec vec2 = vec_new(sizeof(int), 1);
    elem = 3;
    vec_push(&vec2, &elem);
    elem = 4;
    vec_push(&vec2, &elem);
    elem = 5;
    vec_push(&vec2, &elem);


    Vec* combined_vec = vec_insert(&vec1, &vec2, 2);
    assert(combined_vec->size == 7);
    assert(*(int*)vec_get(combined_vec, 0) == 1);
    assert(*(int*)vec_get(combined_vec, 1) == 2);
    assert(*(int*)vec_get(combined_vec, 2) == 3);
    assert(*(int*)vec_get(combined_vec, 3) == 4);
    assert(*(int*)vec_get(combined_vec, 4) == 5);
    assert(*(int*)vec_get(combined_vec, 5) == 6);
    assert(*(int*)vec_get(combined_vec, 6) == 7);
    vec_free(&vec1);
    vec_free(&vec2);
}