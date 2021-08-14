#include "string_helpers.h"

// ======================== String Vector =============================

// Create a new string vector
StrVector str_vec_new(int initial_size) {
    StrVector vec;
    vec.size = 0;
    vec.elems = malloc(initial_size*sizeof(char*));
    vec.max_size = initial_size;
    return vec;
}

StrVector* str_vec_new_ptr(int initial_size) {
    StrVector* vec = malloc(sizeof(StrVector));
    vec->size = 0;
    vec->elems = malloc(initial_size*sizeof(char*));
    vec->max_size = initial_size;
    return vec;
}

char* str_copy(char *str) {
    return str_substr(str, strlen(str));
}

// Copy a substring of a string. Remember to free it later!
char* str_substr(const char* string, int length)
{
    // +5 is required to prevent memory leaks, why?
	char *output_string = calloc(length+5, sizeof(char));
	memcpy(output_string, string, length*sizeof(char));
    //*(output_string+length) = '\0';
	return output_string;
}

// Add an element to the end of the vector
void str_vec_push(StrVector* str_vec, char* str) {
    str_vec->size++;
    if (str_vec->size >= str_vec->max_size) {
        str_vec_realloc(str_vec, str_vec->max_size*2);
    }
    str_vec->elems[str_vec->size-1] = str_copy(str);
}

// Add an element to the end of the vector
void str_vec_push_no_copy(StrVector* str_vec, char* str) {
    str_vec->size++;
    if (str_vec->size >= str_vec->max_size) {
        str_vec_realloc(str_vec, str_vec->max_size*2);
    }
    str_vec->elems[str_vec->size-1] = str;
}

// Reallocate the vector size
void str_vec_realloc(StrVector* str_vec, int new_size) {
    str_vec->elems = realloc(str_vec->elems, new_size*sizeof(char*));
    str_vec->max_size = new_size;
}

// Free the vector memory
void str_vec_free(StrVector* str_vec) {
    for (int i = 0; i < str_vec->size; i++) {
        free(str_vec->elems[i]);
    }
    free(str_vec->elems);
}

// Find the combined size of the vector items
int str_vec_total_item_size(StrVector* str_vec) {
    int size = 0;
    for (int i = 0; i < str_vec->size; i++) {
        size += strlen(str_vec->elems[i]);
    }
    return size;
}

char* str_vec_join(StrVector* str_vec) {
    int total_size = str_vec_total_item_size(str_vec);
    char *joined_start = calloc(total_size+1, sizeof(char)); // Null terminated
    char *joined_cur = joined_start;
    for (int i = 0; i < str_vec->size; i++) {
        char* str = str_vec->elems[i];
        while (*str != '\0') {
            *joined_cur = *str;
            str++;
            joined_cur++;
        }
    }
    return joined_start;
}

char* str_vec_join_with_delim(StrVector* str_vec, char delim) {
    int total_size = str_vec_total_item_size(str_vec);
    char *joined_start = calloc(total_size+1 + str_vec->size, sizeof(char)); // Null terminated
    char *joined_cur = joined_start;
    for (int i = 0; i < str_vec->size; i++) {
        char* str = str_vec->elems[i];
        while (*str != '\0') {
            *joined_cur = *str;
            str++;
            joined_cur++;
        }
        *joined_cur = delim;
        joined_cur++;
    }
    return joined_start;
}

StrVector str_vec_slice(StrVector* str_vec, int from_index, int to_index) {
    StrVector slice_vec = *str_vec;
    slice_vec.elems += from_index;
    slice_vec.size = to_index - from_index;
    slice_vec.max_size = slice_vec.max_size - from_index;
    return slice_vec;
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

// Split a C string based on whitespace and return a StrVector
StrVector str_split_on_whitespace(char* str) {
    StrVector str_vec = str_vec_new(4);
    char* start = str;
    while (*str != '\0') {
        if (isspace(*str)) {
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

// Add two string vectors
StrVector* str_vec_add(StrVector *str_vec1, StrVector* str_vec2) {
    str_vec1->elems = realloc(str_vec1->elems, (str_vec1->size + str_vec2->size) * sizeof(char*));
    memcpy(str_vec1->elems+str_vec1->size, str_vec2->elems, str_vec2->size*sizeof(char*));
    str_vec1->size = str_vec1->size + str_vec2->size;
    str_vec1->max_size = str_vec1->size;
    free(str_vec2->elems);
    return str_vec1;
}

// Print contents of string vector
void str_vec_print(StrVector* str_vec) {
    for (size_t i = 0; i < str_vec->size; i++)
    {
        printf("%s\n", str_vec->elems[i]);
    }
    
}

// ======================== String Helpers =============================


char* str_add(char* str1, char* str2) {
    if (str1 == NULL) str1 = "";
    if (str2 == NULL) str2 = "";
    int length = strlen(str1) + strlen(str2);
    char* added_str_start = calloc(length+1, sizeof(char));
    char* added_str = added_str_start;
    while(*str1 != '\0') {
        *added_str = *str1;
        added_str++;
        str1++;
    }
    while(*str2 != '\0') {
        *added_str = *str2;
        added_str++;
        str2++;
    }
    return added_str_start;
} 

char* str_multiply(char* str, int n) {
    char* prev_str = calloc(1, sizeof(char));;
    char* new_str = prev_str;
    while(n != 0) {
        new_str = str_add(prev_str, str);
        free(prev_str);
        prev_str = new_str;
        n--;
    }
    return new_str;
}

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
int str_contains(char* str, char* match) {
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
// A word has to end or begin with either whitespace or ({[:.
int str_contains_word(char* str, char* match) {
    char* cur_match_char = match;
    char* match_end = match;
    char* str_start = str;
    bool prev_whitespace = true;
    while(*match_end != '\0') match_end++;
    while (*str != '\0') {
        if ((!isalnum(*str) && *str != '_') || (prev_whitespace && *cur_match_char == *str)) {
            prev_whitespace = true;
        }
        else {
            prev_whitespace = false;
        }
        if (prev_whitespace && *cur_match_char == *str) {
            cur_match_char++;
            if (cur_match_char == match_end) {
                // Check if the last char matches
                str++;
                int index = str - str_start - (match_end - match) + 1;
                char* before_word = str_start + index - 2;
                if (!isalnum(*str) && *str != '_' && (before_word < str_start || !isalnum(*before_word))) {
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

void str_fill(char* str, int length, char c) {
    for (size_t i = 0; i < length; i++) {
        str[i] = c;
    }
}

char* str_strip(char* str) {
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

int str_index_of_reverse(char* str, char c) {
    int str_length = strlen(str);
    for (size_t i = 0; i < str_length; i++)
    {
        if (*(str+str_length-i) == c) {
            return str_length-i;
        }
    }
    return -1;
}

int max(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

int min(int a, int b) {
    if (a < b)
        return a;
    else
        return b;
}