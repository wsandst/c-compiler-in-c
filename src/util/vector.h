// Void pointer vector type
#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RUNTIME_ERROR_HANDLING

typedef struct Vec Vec;

struct Vec {
    int size;
    int max_size;
    int elem_bytes;
    void* elems;
};

// Constructor, create a new Vector object containing elements of size bytes and
// with an initial reserved size of initial_size
Vec vec_new(int bytes, int initial_size);

// Constructor, create a new dynamic Vector object, return the pointer
Vec* vec_new_dyn(int bytes);

// Destructor, Free the memory of the vector
void vec_free(Vec* vec);

// Reserve/reallocate the memory of the vector
void vec_reserve(Vec* vec, int new_max_size);

// Resize the vector and set the new elements to 0
void vec_resize(Vec* vec, int new_size);

// Copy the values of a vector into a new vector
Vec vec_copy(Vec* vec);

// Push an element to the end of the vector
void vec_push(Vec* vec, void* elem);

// Get an element of the vector at index i
void* vec_get(Vec* vec, int i);

// Return the last element of the vector and remove it
void* vec_pop(Vec* vec);

// Return the last element of the vector
void* vec_peek(Vec* vec);

// Insert vec2 into vec1 at a specific index, return vec1
Vec* vec_insert(Vec* vec1, Vec* vec2, int insert_index);

// Return a slice of the vector. from_index inclusive, to_index exclusive
// Do not modify this slice without making a copy, will mess up the original vector
Vec vec_slice(Vec* vec, int from_index, int to_index);

// Runtime vector errors
void vec_error(char* error_message);