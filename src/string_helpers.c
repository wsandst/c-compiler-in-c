#include "string_helpers.h"

// ======================== String Vector =============================

// Create a new string vector
StrVector str_vec_new(int initial_size) {
    StrVector tokens;
    tokens.size = 0;
    tokens.max_size = initial_size;
    tokens.elems = malloc(initial_size*sizeof(char*));
    return tokens;
}

char* str_copy(char *str) {
    return str_substr(str, strlen(str));
}

// Copy a substring of a string. Remember to free it later!
char* str_substr(const char *string, int length)
{
	char *output_string = calloc(length+1, sizeof(char));
	memcpy(output_string, string, length*sizeof(char));
	return output_string;
}

// Add an element to the end of the vector
void str_vec_push(StrVector *str_vec, char* str) {
    str_vec->size++;
    if (str_vec->size >= str_vec->max_size) {
        str_vec_realloc(str_vec, str_vec->max_size*2);
    }
    str_vec->elems[str_vec->size] = str_substr(str, strlen(str));
}

// Add an element to the end of the vector
void str_vec_push_no_copy(StrVector *str_vec, char* str) {
    str_vec->size++;
    if (str_vec->size >= str_vec->max_size) {
        str_vec_realloc(str_vec, str_vec->max_size*2);
    }
    str_vec->elems[str_vec->size-1] = str;
}

// Reallocate the vector size
void str_vec_realloc(StrVector *str_vec, int new_size) {
    str_vec->elems = realloc(str_vec->elems, new_size*sizeof(char*));
    str_vec->max_size = new_size;
}

// Free the vector memory
void str_vec_free(StrVector *str_vec) {
    for (int i = 0; i < str_vec->size+1; i++) {
        free(str_vec->elems[i]);
    }
    free(str_vec->elems);
}

// Split a C string based on a delimiter and return a StrVector
StrVector str_split(char* str, char delimiter) {
    StrVector str_vec = str_vec_new(4);
    char* start = str;
    while (*str != '\0') {
        if (*str == delimiter) {
            if (start != str) {
                int length = str - start;
                char* word = str_substr(start, length);
                str_vec_push_no_copy(&str_vec, word);
            }
            start = str + 1;
        }
        str++;
    }
    if (start != str) {
        char* word = str_substr(start, str - start);
        str_vec_push_no_copy(&str_vec, word);
    }
    return str_vec;
}

void str_vec_print(StrVector* str_vec) {
    for (size_t i = 0; i < str_vec->size; i++)
    {
        printf("%s\n", str_vec->elems[i]);
    }
    
}

// ======================== String Helpers =============================

// Does the string start with the string provided? Return 0 if not,
// else return the index
bool str_startswith(char* str, char* match) {
    while (*str == *match || *match == '\0' || *str == '\0') {
        if (*match == '\0' || *str == '\0') {
            return true;
        }
        match++;
        str++;
    }
    return false;
}

// Does the string end with the string provided?
bool str_endswith(char* str, char* match) {
    char* match_start = match;
    char* str_start = str;
    while (*match != '\0') match++;
    while (*str != '\0') str++;
    match--;
    str--;
    while (*str == *match) {
        if (match == match_start || str == str_start) {
            return true;
        }
        match--;
        str--;
    }
    return false;
}

// Does the string contain the string provided? 
// If true, return the index+1, else return 0
int str_contains(char *str, char* match) {
    char* cur_match_char = match;
    char* match_end = match;
    char* str_start = str;
    while(*match_end != '\0') match_end++;
    while (*str != '\0') {
        if (*cur_match_char == *str) {
            cur_match_char++;
            if (cur_match_char == match_end) {
                return str - str_start - (match_end - match) + 2;
            }
        }
        else {
            cur_match_char = match; 
        }
        str++;
    }
    return 0;
}

// Does the string contain the string word provided?
// A word has to end with either whitespace or ({[:.
int str_contains_word(char *str, char* match) {
    char* cur_match_char = match;
    char* match_end = match;
    char* str_start = str;
    while(*match_end != '\0') match_end++;
    while (*str != '\0') {
        if (*cur_match_char == *str) {
            cur_match_char++;
            if (cur_match_char == match_end) {
                // Check if the last char matches
                str++;
                int index = str - str_start - (match_end - match) + 1;
                char* before_word = str_start + index - 2;
                if (!isalnum(*str) && (before_word < str_start || !isalnum(*before_word))) {
                    return index;
                }
                else {
                    cur_match_char = match; 
                }
                
            }
        }
        else {
            cur_match_char = match; 
        }
        str++;
    }
    return 0;
}

void str_fill(char *str, int length, char c) {
    for (size_t i = 0; i < length; i++) {
        str[i] = c;
    }
}

char* str_strip(char *str) {
    int length = strlen(str);
    char *start = str;
    char *end = str+length-1;
    while ((*start == ' ' || *start == '\n' || *start == '\t') && end != start) {
        start++;
    }
    while ((*end == ' ' || *end == '\n' || *end == '\t') && end != start) {
        end--;
    }
    return str_substr(start, end - start+1);
}