#include "vector.h"

Vec vec_new(int bytes) {
    Vec vec;
    vec.elems = calloc(1, bytes);
    vec.elem_bytes = bytes;
    vec.size = 0;
    vec.max_size = 1;
    return vec;
}

void vec_free(Vec* vec) {
    free(vec->elems);
}

void vec_realloc(Vec* vec, int new_max_size) {
    vec->elems = realloc(vec->elems, vec->elem_bytes*new_max_size);
    vec->max_size = new_max_size;
}

void vec_push(Vec* vec, void* elem) {
    vec->size++;
    if (vec->size > vec->max_size) {
        vec_realloc(vec, vec->max_size*2);
    }
    memcpy((char*) (vec->elems) + (vec->size-1)*vec->elem_bytes, elem, vec->elem_bytes);
}

void* vec_get(Vec* vec, int i) {
    return (void*)(((char*)vec->elems) + i*(vec->elem_bytes));
}