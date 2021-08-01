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

Vec vec_new(int bytes);

void vec_realloc(Vec* vec, int new_max_size);

void vec_push(Vec* vec, void* elem);

void vec_free(Vec* vec);

void* vec_get(Vec* vec, int i);