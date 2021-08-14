#include "vector.h"

Vec vec_new(int bytes, int initial_size) {
    Vec vec;
    vec.elems = calloc(1, bytes*initial_size);
    vec.elem_bytes = bytes;
    vec.size = 0;
    vec.max_size = initial_size;
    return vec;
}

Vec* vec_new_dyn(int bytes) {
    Vec* vec = calloc(1, sizeof(Vec));
    *vec = vec_new(bytes, 1);
    return vec;
}

void vec_free(Vec* vec) {
    free(vec->elems);
}

void vec_reserve(Vec* vec, int new_max_size) {
    vec->elems = realloc(vec->elems, vec->elem_bytes*new_max_size);
    // Zero new memory
    memset(((char*)vec->elems)+vec->max_size*vec->elem_bytes, 0, vec->elem_bytes*(new_max_size-vec->max_size));
    vec->max_size = new_max_size;
}

void vec_resize(Vec* vec, int new_size) {
    if (new_size - vec->max_size > 0) { // Past max_size, realloc required
        vec->elems = realloc(vec, new_size * 1.25);
    }
    vec->size = new_size;
}

Vec vec_copy(Vec* vec) {
    Vec vec_copy = vec_new(vec->elem_bytes, vec->max_size);
    vec_copy.size = vec->size;
    memcpy(vec_copy.elems, vec->elems, vec->elem_bytes*vec->size);
    return vec_copy;
}

void vec_push(Vec* vec, void* elem) {
    vec->size++;
    if (vec->size > vec->max_size) {
        vec_reserve(vec, vec->max_size*2);
    }
    memcpy((char*) (vec->elems) + (vec->size-1)*vec->elem_bytes, elem, vec->elem_bytes);
}

void* vec_get(Vec* vec, int i) {
    #ifdef RUNTIME_ERROR_CHECKS
    if (vec.size <= i) {
        vec_error("vec_get: Index out of range of vector size!");
    }
    #endif
    return (void*)(((char*)vec->elems) + i*(vec->elem_bytes));
}

// Return the last element of the vector and remove it
void* vec_pop(Vec* vec) {
    #ifdef RUNTIME_ERROR_CHECKS
    if (vec.size == 0) {
        vec_error("vec_pop: Attempted to pop empty vector!");
    }
    #endif
    vec->size--;
    return vec->elems + vec->size * vec->elem_bytes;
}

// Return the last element of the vector
void* vec_peek(Vec* vec) {
    return vec->elems + (vec->size - 1) * vec->elem_bytes;
}

Vec* vec_insert(Vec* vec1, Vec* vec2, int insert_index) {
    #ifdef RUNTIME_ERROR_CHECKS
    if (vec1->elem_bytes != vec2->elem_bytes) {
        vec_error("vec_insert: Vectors have different element types!");
    }
    #endif
    int new_size = vec1->size + vec2->size;
    vec1->elems = realloc(vec1->elems, new_size*vec1->elem_bytes);
    // Shift the latter half of vec1 to the end
    int to_index = new_size-(vec1->size - insert_index);
    void* from_address = vec1->elems+insert_index*vec1->elem_bytes;
    memmove(vec1->elems + to_index*vec1->elem_bytes, from_address, (vec1->size - insert_index)*(vec1->elem_bytes));
    // Copy in vec2
    memcpy(from_address, vec2->elems, vec2->size*(vec2->elem_bytes));
    vec1->size = new_size;
    vec1->max_size = new_size;
    
    return vec1;
}

// Return a slice of the vector. from_index inclusive, to_index exclusive
Vec vec_slice(Vec* vec, int from_index, int to_index) {
    #ifdef RUNTIME_ERROR_CHECKS
    if (from_index >= vec->size || to_index > vec->size) {
        vec_error("vec_slice: Index out of range of vector size!");
    }
    #endif
    Vec slice_vec = *vec;
    slice_vec.elems += from_index;
    slice_vec.size = to_index - from_index;
    slice_vec.max_size = slice_vec.max_size - from_index;
    return slice_vec;
}

void vec_error(char* error_message) {
    fprintf(stderr, "Vector error: %s\n", error_message);
    exit(1); 
}