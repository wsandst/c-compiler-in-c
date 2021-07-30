#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
Contains a string vector and various string helpers
TODO: Contains word does not check the start of a word, if it is a space or not etc. This is bad!
*/

// ======================== String Vector =============================

// Represents a String Vector (reallocating array)
struct StrVector {
    char **elems;
    int size;
    int max_size;
};

typedef struct StrVector StrVector;

// Create a new StrVector
StrVector str_vec_new(int initial_size);

// Add an element, a C string, to the end of the vector
void str_vec_push(StrVector* str_vec, char* str);

void str_vec_push_no_copy(StrVector* str_vec, char* str);

// Reallocate the vector size
void str_vec_realloc(StrVector* str_vec, int new_size);

// Free the vector memory and the containing strings?
void str_vec_free(StrVector* str_vec);

// Print the C strings in the vector
void str_vec_print(StrVector* str_vec);

// Join a string vector into a single C string
char* str_vec_join(StrVector* str_vec);

// Add two string vectors, store result in str_vec1 and free str_vec2
StrVector* str_vec_add(StrVector *str_vec1, StrVector* str_vec2);

// Split a C string based on a delimiter and return a StrVector
StrVector str_split(char* str, char delimiter);

// ======================== String Helpers =============================
// IMPORTANT: These all work on C strings. Without a terminating NULL character,
// many of these functions will crash or loop forever!

// Copy a C String
char* str_copy(char* str);

// Copy a C String substring
char* str_substr(const char* string, int length);

// Add str1 to str2
char* str_add(char* str1, char* str2);

// Add str to itself, n times
char* str_multiply(char *str, int n);

// Does the string start with the string provided?
bool str_startswith(char* str, char* match);

// Does the string end with the string provided?
bool str_endswith(char* str, char* match);

// Does the string contain the string provided?
int str_contains(char* str, char* match);

// Does the string contain the string word provided?
// A word has to end with either whitespace or ({[:.
int str_contains_word(char* str, char* match);

// Fill a str to char c, inplace. Has to be a mutable str!
void str_fill(char* str, int length, char c);

// Strip all whitespace from the start and the end of string
char* str_strip(char* str);

int max(int a, int b);