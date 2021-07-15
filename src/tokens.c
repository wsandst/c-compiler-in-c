#include "tokens.h"

/*
TODO: Implement handling of comments in strings and reverse
Left to tokenize: 
*/

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

    //str_vec_print(&lines);
    

    tokenize_preprocessor(&tokens, &lines);
    tokenize_comments(&tokens, &lines);
    tokenize_strings(&tokens, &lines);
    tokenize_keywords(&tokens, &lines);
    tokenize_ops(&tokens, &lines);
    tokenize_idents(&tokens, &lines);

    str_vec_print(&lines);

    //tokens_print(&tokens);

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
    tokens->size = token_count;
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
    int comment_src_pos;
    char* zero_from;
    bool zero_from_start = false;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str = str_split->elems[i];

        // Block comments
        int block_comment_index = str_contains(str, "/*");
        if (block_comment_index) {
            if (!seeking_block_comment_end) {
                zero_from = str + block_comment_index - 1;
                comment_src_pos = src_pos + block_comment_index - 1;
            }
            seeking_block_comment_end = true;
        }
        if (seeking_block_comment_end) {
            int block_comment_end = str_contains(str, "*/");
            if (block_comment_end) {
                // Found the end
                if (zero_from_start) {
                    str_fill(str, block_comment_end + 1, ' ');
                }
                else {
                    str_fill(zero_from, zero_from - str + block_comment_end + 1, ' ');
                }
                tokens->elems[comment_src_pos].type = TK_COMMENT;
                tokens->elems[comment_src_pos].data.string = "BLOCK COMMENT N/A";
                seeking_block_comment_end = false;
                zero_from_start = false;
            }
            else if (!zero_from_start) {
                // This is all a block comment, zero out the contents
                str_fill(zero_from, zero_from - str, ' ');
                zero_from_start = true;
                continue;
            }
            else {
                str_fill(str, strlen(str), ' ');
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

void tokenize_keywords(Tokens* tokens, StrVector *str_split) {
    tokenize_keyword(tokens, str_split, "while", TK_WHILE);
    tokenize_keyword(tokens, str_split, "do", TK_DO);
    tokenize_keyword(tokens, str_split, "if", TK_IF);
    tokenize_keyword(tokens, str_split, "else", TK_ELSE);
    tokenize_keyword(tokens, str_split, "for", TK_FOR);
    tokenize_keyword(tokens, str_split, "break", TK_BREAK);
    tokenize_keyword(tokens, str_split, "continue", TK_CONTINUE);
    tokenize_keyword(tokens, str_split, "return", TK_RETURN);
    tokenize_keyword(tokens, str_split, "switch", TK_SWITCH);
    tokenize_keyword(tokens, str_split, "case", TK_CASE);
    tokenize_keyword(tokens, str_split, "default", TK_DEFAULT);
    tokenize_keyword(tokens, str_split, "goto", TK_GOTO);
    tokenize_keyword(tokens, str_split, "label", TK_LABEL);
    tokenize_keyword(tokens, str_split, "typedef", TK_TYPEDEF);
    tokenize_keyword(tokens, str_split, "const", TK_CONST);
    tokenize_keyword(tokens, str_split, "long", TK_LONG);
    tokenize_keyword(tokens, str_split, "short", TK_SHORT);
    tokenize_keyword(tokens, str_split, "signed", TK_SIGNED);
    tokenize_keyword(tokens, str_split, "unsigned", TK_UNSIGNED);
    tokenize_keyword(tokens, str_split, "struct", TK_STRUCT);
    tokenize_keyword(tokens, str_split, "union", TK_UNION);
    tokenize_keyword(tokens, str_split, "int", TK_INT);
    tokenize_keyword(tokens, str_split, "float", TK_FLOAT);
    tokenize_keyword(tokens, str_split, "double", TK_DOUBLE);
    tokenize_keyword(tokens, str_split, "char", TK_CHAR);
    tokenize_keyword(tokens, str_split, "void", TK_VOID);
}

void tokenize_keyword(Tokens* tokens, StrVector *str_split, char* keyword, enum KeywordType type) {
    int src_pos = 0;
    int keyword_length = strlen(keyword);
    for (size_t i = 0; i < str_split->size; i++)
    {
        char* str = str_split->elems[i];
        int match_i = 0;
        while ((match_i = str_contains_word(str, keyword))) {
            char* start = str + match_i-1;
            int keyword_src_index = src_pos+match_i-1;
            str_fill(start, keyword_length, ' ');
            tokens->elems[keyword_src_index].type = TK_KEYWORD;
            tokens->elems[keyword_src_index].sub_type.keyword = type;
            tokens->elems[keyword_src_index].data.string = keyword;
        }
        src_pos += strlen(str);
    }
}

void tokenize_ops(Tokens* tokens, StrVector *str_split) {
    // Similar to keywords tokenization, except we want to isolate words
    // which only have >=<
    tokenize_op(tokens, str_split, "||", TK_OR);
    tokenize_op(tokens, str_split, "&&", TK_AND);
    tokenize_op(tokens, str_split, ">>", TK_RIGHTSHIFT);
    tokenize_op(tokens, str_split, "<<", TK_LEFTSHIFT);
    tokenize_op(tokens, str_split, "==", TK_EQ);
    tokenize_op(tokens, str_split, "!=", TK_NEQ);
    tokenize_op(tokens, str_split, "**", TK_EXP);
    tokenize_op(tokens, str_split, ">=", TK_GTE);
    tokenize_op(tokens, str_split, "<=", TK_LTE);
    tokenize_op(tokens, str_split, "+", TK_PLUS);
    tokenize_op(tokens, str_split, "-", TK_MINUS);
    tokenize_op(tokens, str_split, "*", TK_MULT);
    tokenize_op(tokens, str_split, "/", TK_DIV);
    tokenize_op(tokens, str_split, "%", TK_MOD);
    tokenize_op(tokens, str_split, "|", TK_BITOR);
    tokenize_op(tokens, str_split, "&", TK_BITAND);
    tokenize_op(tokens, str_split, "~", TK_COMPL);
    tokenize_op(tokens, str_split, "^", TK_XOR);
    tokenize_op(tokens, str_split, ">", TK_GT);
    tokenize_op(tokens, str_split, "<", TK_LT);
    tokenize_op(tokens, str_split, "!", TK_NOT);
    tokenize_op(tokens, str_split, "=", TK_ASSIGN);
    tokenize_op(tokens, str_split, "?", TK_QST);
}

void tokenize_op(Tokens* tokens, StrVector *str_split, char* op, enum OpType type) {
    int src_pos = 0;
    int op_length = strlen(op);
    for (size_t i = 0; i < str_split->size; i++)
    {
        char* str = str_split->elems[i];
        int match_i = 0;
        while ((match_i = str_contains(str, op))) {
            char* start = str + match_i-1;
            int keyword_src_index = src_pos+match_i-1;
            str_fill(start, op_length, ' ');
            tokens->elems[keyword_src_index].type = TK_OP;
            tokens->elems[keyword_src_index].sub_type.keyword = type;
            tokens->elems[keyword_src_index].data.string = op;
        }
        src_pos += strlen(str);
    } 
}

void tokenize_idents(Tokens *tokens, StrVector *str_split) {
    // Find words (separated by non-alphanum characters) which
    // starts with an alpha char or _, made up of only alphanum characters
    // Make a new version of str_contains_word? str_contains_alpha_word?
    int src_pos = 0;
    char* ident_start;
    bool matching_ident = false;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str_start = str_split->elems[i];
        char* str = str_start;
        bool prev_is_whitespace = true;
        while (*str != '\0') {
            if (!matching_ident) {
                if ((isalpha(*str) || *str == '_') && prev_is_whitespace) { // Found start of identifier
                    matching_ident = true;
                    ident_start = str;
                }
            }
            else {
                if (!isalnum(*str)) { // Found end of identifier
                    int length = str - ident_start;
                    tokens->elems[src_pos-length].type = TK_IDENT;
                    tokens->elems[src_pos-length].data.string = str_substr(ident_start, length);
                    str_fill(ident_start, length, ' ');
                    matching_ident = false;
                }
            }
            if (!isalnum(*str) && *str != '_') {
                prev_is_whitespace = true;
            }
            else {
                prev_is_whitespace = false;
            }
            str++;
            src_pos++;
        }
        if (matching_ident) { // Found end of identifier, line ended
            int length = str - ident_start;
            tokens->elems[src_pos-length].type = TK_IDENT;
            tokens->elems[src_pos-length].data.string = str_substr(ident_start, length);
            str_fill(ident_start, length, ' ');
            matching_ident = false;
        }
    }
}

void tokens_print(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token t = tokens->elems[i];
        if (t.type == TK_NONE) {
            continue;
        }
        printf("[T:%i, TS:%i ", t.type, t.sub_type.keyword);
        if (t.type == TK_COMMENT || t.type == TK_PREPROCESSOR) {
            printf("V: %s", t.data.string);
        } 
        else if (t.type == TK_LITERAL) {
            if (t.sub_type.literal == TK_LSTRING) {
                printf("V: \"%s\"", t.data.string);
            }
            else if (t.sub_type.literal == TK_LCHAR) {
                printf("V: \'%s\'", t.data.string);
            }
        }
        printf("], \n");
    }
    
}