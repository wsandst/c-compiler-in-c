#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// ======================== String Vector =============================

struct StrVector {
    char **elems;
    int size;
    int max_size;
};

typedef struct StrVector StrVector;

// Create a new string vector
StrVector str_vec_new(int initial_size);

// Copy a C String
char* str_substr(const char *string, int length);

// Add an element to the end of the vector
void str_vec_push(StrVector *str_vec, char* str);

// Reallocate the vector size
void str_vec_realloc(StrVector *str_vec, int new_size);

// Free the vector memory and the containing strings?
void str_vec_free(StrVector *str_vec);

// Split a char array based on delimiter and return a StrVector
StrVector str_split(char* str, char delimiter);

// ======================== String Helpers =============================

// Does the string start with the string provided?
bool str_startswith(char *str, char *match);

// Does the string end with the string provided?
bool str_endswith(char *str, char *match);

// Does the string contain the string provided?
int str_contains(char *str, char *match);