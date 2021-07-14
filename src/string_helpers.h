#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
void str_vec_push(StrVector *str_vec, char* str);

// Reallocate the vector size
void str_vec_realloc(StrVector *str_vec, int new_size);

// Free the vector memory and the containing strings?
void str_vec_free(StrVector *str_vec);

// Print the C strings in the vector
void str_vec_print(StrVector *str_vec);

// Split a C string based on a delimiter and return a StrVector
StrVector str_split(char* str, char delimiter);

// ======================== String Helpers =============================
// IMPORTANT: These all work on C strings. Without a terminating NULL character,
// many of these functions will crash or loop forever!

// Copy a C String
char* str_copy(char *str);

// Copy a C String substring
char* str_substr(const char *string, int length);

// Does the string start with the string provided?
bool str_startswith(char *str, char *match);

// Does the string end with the string provided?
bool str_endswith(char *str, char *match);

// Does the string contain the string provided?
int str_contains(char *str, char *match);

// Does the string contain the string word provided?
// A word has to end with either whitespace or ({[:.
int str_contains_word(char *str, char *match);

// Fill a str to char c, inplace. Has to be a mutable str!
char* str_fill(char *str, int length, char c);

// Strip all whitespace from the start and the end of string
char* str_strip(char *str);