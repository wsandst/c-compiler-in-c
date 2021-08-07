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

void test_vector() {
    printf("[CTEST] Running vector tests...\n");

    int elem = 1;
    Vec vec = vec_new(sizeof(int));
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

    vec_free(&vec);

    // Test with a struct
    TestStruct struct_elem;
    struct_elem.x = 1;
    struct_elem.c = 2;
    vec = vec_new(sizeof(TestStruct));
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

    printf("[CTEST] Passed vector tests!\n");
}