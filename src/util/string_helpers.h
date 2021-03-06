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
    char** elems;
    int size;
    int max_size;
};

typedef struct StrVector StrVector;

// Create a new StrVector
StrVector str_vec_new(int initial_size);
StrVector* str_vec_new_ptr(int initial_size);

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

// Join a string vector into a single C string, separate parts with delimiter
char* str_vec_join_with_delim(StrVector* str_vec, char delim);

// Return a slice of the string vector. from_index inclusive, to_index exclusive
StrVector str_vec_slice(StrVector* str_vec, int from_index, int to_index);

// Add two string vectors, store result in str_vec1 and free str_vec2
StrVector* str_vec_add(StrVector* str_vec1, StrVector* str_vec2);

// Deep copy the string vector
StrVector str_vec_copy(StrVector* str_vec);

// Split a C string based on a delimiter and return a StrVector
StrVector str_split(char* str, char delimiter);

// Split a C string based on whitespace and return a StrVector
StrVector str_split_on_whitespace(char* str);

// Split a C string based on newlines
StrVector str_split_lines(char* str);

// ======================== String Helpers =============================
// IMPORTANT: These all work on C strings. Without a terminating NULL character,
// many of these functions will crash or loop forever!

// Copy a C String
char* str_copy(char* str);

// Copy a C String substring
char* str_substr(char* string, int length);

// Add str1 to str2
char* str_add(char* str1, char* str2);

// Add str to itself, n times
char* str_multiply(char* str, int n);

// Does the string start with the string provided?
bool str_startswith(char* str, char* match);

// Does the string end with the string provided?
bool str_endswith(char* str, char* match);

// Does the string contain the string provided?
int str_contains(char* str, char* match);

// Does the string contain the string word provided?
// A word has to end with either whitespace or ({[:.
int str_contains_word(char* str, char* match);

// Escape the contents of a NASM ` ` string by placing backslashes infront of `
// Frees the input string
char* str_escape_nasm_chars(char* str);

// Fill a str to char c, inplace. Has to be a mutable str!
void str_fill(char* str, int length, char c);

// Strip all whitespace from the start and the end of string
char* str_strip(char* str);

int str_index_of_reverse(char* str, char c);

int max(int a, int b);

int min(int a, int b);

// These are implemented manually because this compiler does not support
// macro functions which is how they are implemented in GCC
bool c_isdigit(char c);

bool c_isalpha(char c);

bool c_isalnum(char c);

bool c_isspace(char c);