// Void pointer vector type
#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Vec Vec;

struct Vec {
    int size;
    int max_size;
    int elem_bytes;
    void* elems;
};

Vec vec_new(int bytes) {
    Vec vec;
    vec.elems = calloc(1, bytes);
    vec.elem_bytes = bytes;
    vec.size = 0;
    vec.max_size = 1;
    return vec;
}

void vec_realloc(Vec* vec, int new_max_size) {
    vec->elems = realloc(vec->elems, vec->elem_bytes*new_max_size);
}

void vec_push(Vec* vec, void* elem) {
    vec->size++;
    if (vec->size > vec->max_size) {
        vec_realloc(vec, vec->max_size*2);
    }
    memcpy(vec->elems + vec->size, elem, vec->elem_bytes);
}