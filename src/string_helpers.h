#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
Contains a string vector and various string helpers
*/

// ======================== String Vector =============================

struct StrVector {
    char **elems;
    int size;
    int max_size;
};

typedef struct StrVector StrVector;

// Create a new string vector
StrVector str_vec_new(int initial_size);

// Add an element to the end of the vector
void str_vec_push(StrVector *str_vec, char* str);

// Reallocate the vector size
void str_vec_realloc(StrVector *str_vec, int new_size);

// Free the vector memory and the containing strings?
void str_vec_free(StrVector *str_vec);

void str_vec_print(StrVector *str_vec);

// Split a char array based on delimiter and return a StrVector
StrVector str_split(char* str, char delimiter);

// ======================== String Helpers =============================

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

char* str_fill(char *str, int length, char c);

char* str_strip(char *str);