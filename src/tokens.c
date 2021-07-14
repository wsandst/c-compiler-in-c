#include "tokens.h"

Tokens tokenize(char* src) {
    int src_length = strlen(src);
    Tokens tokens = tokens_new(src_length);

    // Start identifying tokens
    StrVector lines = str_split(src, '\n');
    for (size_t i = 0; i < lines.size; i++)
    {
        char* new_str = str_strip(lines.elems[i]);
        free(lines.elems[i]);
        lines.elems[i] = new_str;
    }

    str_vec_print(&lines);
    

    tokenize_preprocessor(&tokens, &lines);
    tokenize_comments(&tokens, &lines);
    tokenize_strings(&tokens, &lines);

    str_vec_print(&lines);

    tokens_print(&tokens);

    tokens_trim(&tokens);
    return tokens;
}

// Create a new Tokens object
Tokens tokens_new(int size) {
    Tokens tokens;
    tokens.size = size;
    tokens.elems = calloc(size, sizeof(Token));
    return tokens;
}

void tokens_free(Tokens *tokens) {
    free(tokens->elems);
}

// Remove NULL elements from the Token Array
void tokens_trim(Tokens *tokens) {
    // Count non-none tokens
    int token_count = 0;
    for (size_t i = 0; i < tokens->size; i++) {
        if (tokens->elems[i].type != TK_NONE) {
            token_count++;
        }
    }
    Token* new_token_array = malloc(token_count*sizeof(Token));
    tokens->size = token_count;
    // Add the tokens
    int j = 0;
    for (size_t i = 0; i < tokens->size; i++) {
        Token token = tokens->elems[i];
        if (token.type != TK_NONE) {
            new_token_array[j] = token;
            j++;
        }
    }
    free(tokens->elems);
    tokens->elems = new_token_array;
}

void tokenize_preprocessor(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str = str_split->elems[i];
        // Is this a preprocessor line?
        if (str_startswith(str, "#")) {
            tokens->elems[src_pos].type = TK_PREPROCESSOR;
            tokens->elems[src_pos].data.string = str_copy(str);
            str_fill(str, strlen(str), ' ');
        }
        src_pos += strlen(str);
    }
}

// We do not handle the edge case of a // or /* */ inside a string literal ""
/* */
void tokenize_comments(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    bool seeking_block_comment_end = false;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str = str_split->elems[i];

        // Block comments
        int block_comment_index = str_contains(str, "/*");
        char* zero_from = str;
        if (block_comment_index) {
            if (!seeking_block_comment_end) {
                zero_from = str + block_comment_index - 1;
            }
            seeking_block_comment_end = true;
        }
        if (seeking_block_comment_end) {
            int block_comment_end = str_contains(str, "*/");
            if (block_comment_end) {
                // Found the end
                seeking_block_comment_end = false;
                str_fill(zero_from, block_comment_end+1, ' ');
                int comment_src_pos = src_pos + str - zero_from;
                tokens->elems[comment_src_pos].type = TK_COMMENT;
                tokens->elems[comment_src_pos].data.string = "BLOCK COMMENT N/A";
            }
            else {
                // This is all a block comment, zero out the contents
                str_fill(zero_from, strlen(str), ' ');
                continue;
            }
        }

        // Single line comments
        int comment_index = str_contains(str, "//");
        if (comment_index) {
            int comment_src_pos = src_pos + comment_index + 1;
            char* comment_start = str + comment_index - 1;
            tokens->elems[comment_src_pos].type = TK_COMMENT;
            tokens->elems[comment_src_pos].data.string = str_substr(comment_start, strlen(comment_start));
            str_fill(comment_start, strlen(comment_start), ' ');
        }
        src_pos += strlen(str);
    }
}

void tokenize_strings(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str = str_split->elems[i];

        // Chars, single quotes ''
        int s_quote_start = 1;
        while (s_quote_start) {
            s_quote_start = str_contains(str, "\'");
            if (s_quote_start) {
                char* search_str = str + s_quote_start;
                while (*search_str != '\'' || (search_str > str && (*(search_str-1) == '\\'))) {
                    search_str++;
                }
                int string_src_pos = src_pos + s_quote_start + 1;
                tokens->elems[string_src_pos].type = TK_LITERAL;
                tokens->elems[string_src_pos].sub_type.literal = TK_LCHAR;
                tokens->elems[string_src_pos].data.string = str_substr(str+s_quote_start, search_str - (str + s_quote_start));
                str_fill(str+s_quote_start-1, search_str - (str + s_quote_start) + 2, ' ');
            }
        }
        // Strings, double quotes ""
        int quote_start = 1;
        while (quote_start) {
            quote_start = str_contains(str, "\"");
            if (quote_start) {
                char* search_str = str + quote_start;
                while (*search_str != '\"' || (search_str > str && (*(search_str-1) == '\\'))) {
                    search_str++;
                }
                int string_src_pos = src_pos + quote_start + 1;
                tokens->elems[string_src_pos].type = TK_LITERAL;
                tokens->elems[string_src_pos].sub_type.literal = TK_LSTRING;
                tokens->elems[string_src_pos].data.string = str_substr(str+quote_start, search_str - (str + quote_start));
                str_fill(str+quote_start-1, search_str - (str + quote_start) + 2, ' ');
            }
        }
        src_pos += strlen(str);
    }
}

void tokens_print(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token t = tokens->elems[i];
        if (t.type == TK_NONE) {
            continue;
        }
        printf("[T:%i, TS:%i ", t.type, t.sub_type);
        if (t.type == TK_COMMENT || t.type == TK_PREPROCESSOR) {
            printf("V: %s", t.data.string);
        } 
        else if (t.type == TK_LITERAL) {
            if (t.sub_type.literal == TK_LSTRING) {
                printf("V: \"%s\"", t.data.string);
            }
            else if (t.sub_type.literal == TK_CHAR) {
                printf("V: \'%s\'", t.data.string);
            }
        }
        printf("], \n");
    }
    
}