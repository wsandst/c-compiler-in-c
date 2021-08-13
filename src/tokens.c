#include "tokens.h"

/*
TODO: Implement handling of comments in strings and reverse
Implement tokenization of Hex int values
I don't handle -> correctly. Treat as operator? Or delimiter?
*/

Tokens tokenize(char* src) {
    int src_length = strlen(src)+1;
    Tokens tokens = tokens_new(src_length);

    // Split up src in lines
    StrVector lines = str_split(src, '\n');
    for (size_t i = 0; i < lines.size; i++)
    {
        char* new_str = str_strip(lines.elems[i]);
        free(lines.elems[i]);
        lines.elems[i] = new_str;
    }

    // Set last token to EOF
    tokens_get(&tokens, tokens.size-1)->type = TK_EOF;

    //str_vec_print(&lines);
    
    tokenize_preprocessor(&tokens, &lines);
    tokenize_comments(&tokens, &lines);
    tokenize_strings(&tokens, &lines);
    tokenize_keywords(&tokens, &lines);
    tokenize_ops(&tokens, &lines);
    tokenize_idents(&tokens, &lines);
    tokenize_values(&tokens, &lines);
    tokenize_delims(&tokens, &lines);

    //str_vec_print(&lines);

    //tokens_print(&tokens);

    tokens_trim(&tokens);

    str_vec_free(&lines);
    return tokens;
}

// Create a new Tokens object
Tokens tokens_new(int size) {
    Tokens tokens;
    tokens.size = size;
    tokens.elems = vec_new(sizeof(Token), size);
    vec_resize(&tokens.elems, size);
    return tokens;
}

void tokens_free(Tokens *tokens) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token* t = tokens_get(tokens, i);
        if(t->requires_string_free) {
            // We get invalid frees here for some reason.
            free(t->string_repr);
        }
    }
    vec_free(&tokens->elems);
}

Token* tokens_get(Tokens *tokens, int i) {
    return (Token*) vec_get(&tokens->elems, i);
}

// Remove NULL elements from the Token Array
void tokens_trim(Tokens *tokens) {
    // Count non-none tokens
    int token_count = 0;
    for (size_t i = 0; i < tokens->size; i++) {
        if (tokens_get(tokens, i)->type != TK_NONE) {
            token_count++;
        }
    }
    Vec new_token_vec = vec_new(sizeof(Token), token_count);
    vec_resize(&new_token_vec, token_count);
    // Add the tokens
    int j = 0;
    for (size_t i = 0; i < tokens->size; i++) {
        Token* token = tokens_get(tokens, i);
        if (token->type != TK_NONE) {
            Token* new_token = vec_get(&new_token_vec, j);
            *new_token = *token;
            j++;
        }
    }
    vec_free(&tokens->elems);
    tokens->size = token_count;
    tokens->elems = new_token_vec;
}

// Insert the entire tokens2 into tokens1 at a specific index in tokens1
Tokens* tokens_insert(Tokens* tokens1, Tokens* tokens2, int tokens1_index) {
    vec_insert(&tokens1->elems, &tokens2->elems, tokens1_index);
    tokens1->size = tokens1->elems.size;
    return tokens1;
}

void tokenize_preprocessor(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str = str_split->elems[i];
        // Is this a preprocessor line?
        if (str_startswith(str, "#")) {
            Token* t = tokens_get(tokens, src_pos);
            t->type = TK_PREPROCESSOR;
            t->value.string = str_copy(str);
            t->string_repr = t->value.string;
            t->requires_string_free = true;
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
                Token* t = tokens_get(tokens, comment_src_pos);
                t->type = TK_COMMENT;
                t->string_repr = "BLOCK COMMENT N/A";
                seeking_block_comment_end = false;
                zero_from_start = false;
            }
            else if (!zero_from_start) {
                // Zero from zero_from to the end of the str
                str_fill(zero_from, str - zero_from + strlen(str), ' ');
                zero_from_start = true;
                src_pos += strlen(str);
                continue;
            }
            else {
                // This is all a block comment, zero out the contents
                str_fill(str, strlen(str), ' ');
                src_pos += strlen(str);
                continue;
            }
        }

        // Single line comments
        int comment_index = str_contains(str, "//");
        if (comment_index) {
            int comment_src_pos = src_pos + comment_index + 1;
            char* comment_start = str + comment_index - 1;
            Token* t = tokens_get(tokens, comment_src_pos);
            t->type = TK_COMMENT;
            t->value.string = str_substr(comment_start, strlen(comment_start));
            t->string_repr = t->value.string;
            t->requires_string_free = true;
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
                Token* t = tokens_get(tokens, string_src_pos);
                t->type = TK_LCHAR;
                t->value.string = str_substr(str+s_quote_start, search_str - (str + s_quote_start));
                t->string_repr = t->value.string;
                t->requires_string_free = true;
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
                Token* t = tokens_get(tokens, string_src_pos);
                t->type = TK_LSTRING;
                t->value.string = str_substr(str+quote_start, search_str - (str + quote_start));
                t->string_repr = t->value.string;
                t->requires_string_free = true;
                str_fill(str+quote_start-1, search_str - (str + quote_start) + 2, ' ');
            }
        }
        src_pos += strlen(str);
    }
}

void tokenize_keywords(Tokens* tokens, StrVector *str_split) {
    tokenize_keyword(tokens, str_split, "while", TK_KW_WHILE);
    tokenize_keyword(tokens, str_split, "do", TK_KW_DO);
    tokenize_keyword(tokens, str_split, "if", TK_KW_IF);
    tokenize_keyword(tokens, str_split, "else", TK_KW_ELSE);
    tokenize_keyword(tokens, str_split, "for", TK_KW_FOR);
    tokenize_keyword(tokens, str_split, "break", TK_KW_BREAK);
    tokenize_keyword(tokens, str_split, "continue", TK_KW_CONTINUE);
    tokenize_keyword(tokens, str_split, "return", TK_KW_RETURN);
    tokenize_keyword(tokens, str_split, "switch", TK_KW_SWITCH);
    tokenize_keyword(tokens, str_split, "case", TK_KW_CASE);
    tokenize_keyword(tokens, str_split, "default", TK_KW_DEFAULT);
    tokenize_keyword(tokens, str_split, "goto", TK_KW_GOTO);
    tokenize_keyword(tokens, str_split, "label", TK_KW_LABEL);
    tokenize_keyword(tokens, str_split, "typedef", TK_KW_TYPEDEF);
    tokenize_keyword(tokens, str_split, "const", TK_KW_CONST);
    tokenize_keyword(tokens, str_split, "long", TK_KW_LONG);
    tokenize_keyword(tokens, str_split, "short", TK_KW_SHORT);
    tokenize_keyword(tokens, str_split, "signed", TK_KW_SIGNED);
    tokenize_keyword(tokens, str_split, "unsigned", TK_KW_UNSIGNED);
    tokenize_keyword(tokens, str_split, "struct", TK_KW_STRUCT);
    tokenize_keyword(tokens, str_split, "union", TK_KW_UNION);
    tokenize_keyword(tokens, str_split, "int", TK_KW_INT);
    tokenize_keyword(tokens, str_split, "float", TK_KW_FLOAT);
    tokenize_keyword(tokens, str_split, "double", TK_KW_DOUBLE);
    tokenize_keyword(tokens, str_split, "char", TK_KW_CHAR);
    tokenize_keyword(tokens, str_split, "void", TK_KW_VOID);
}

void tokenize_keyword(Tokens* tokens, StrVector *str_split, char* keyword, enum TokenType type) {
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
            Token* t = tokens_get(tokens, keyword_src_index);
            t->type = type;
            t->string_repr = keyword;
        }
        src_pos += strlen(str);
    }
}

void tokenize_ops(Tokens* tokens, StrVector *str_split) {
    // Similar to keywords tokenization, except we want to isolate words
    tokenize_op(tokens, str_split, "sizeof", TK_OP_SIZEOF);
    tokenize_op(tokens, str_split, "+=", TK_OP_ASSIGN_ADD);
    tokenize_op(tokens, str_split, "-=", TK_OP_ASSIGN_SUB);
    tokenize_op(tokens, str_split, "*=", TK_OP_ASSIGN_MULT);
    tokenize_op(tokens, str_split, "/=", TK_OP_ASSIGN_DIV);
    tokenize_op(tokens, str_split, "%=", TK_OP_ASSIGN_MOD);
    tokenize_op(tokens, str_split, "<<=", TK_OP_ASSIGN_LEFTSHIFT);
    tokenize_op(tokens, str_split, ">>=", TK_OP_ASSIGN_RIGHTSHIFT);
    tokenize_op(tokens, str_split, "&=", TK_OP_ASSIGN_BITAND);
    tokenize_op(tokens, str_split, "|=", TK_OP_ASSIGN_BITOR);
    tokenize_op(tokens, str_split, "^=", TK_OP_ASSIGN_BITXOR);

    tokenize_op(tokens, str_split, "||", TK_OP_OR);
    tokenize_op(tokens, str_split, "&&", TK_OP_AND);
    tokenize_op(tokens, str_split, "==", TK_OP_EQ);
    tokenize_op(tokens, str_split, "!=", TK_OP_NEQ);
    tokenize_op(tokens, str_split, ">=", TK_OP_GTE);
    tokenize_op(tokens, str_split, "<=", TK_OP_LTE);

    tokenize_op(tokens, str_split, ">>", TK_OP_RIGHTSHIFT);
    tokenize_op(tokens, str_split, "<<", TK_OP_LEFTSHIFT);

    tokenize_op(tokens, str_split, "++", TK_OP_INCR);
    tokenize_op(tokens, str_split, "--", TK_OP_DECR);

    tokenize_op(tokens, str_split, "=", TK_OP_ASSIGN);
    tokenize_op(tokens, str_split, "+", TK_OP_PLUS);
    tokenize_op(tokens, str_split, "-", TK_OP_MINUS);
    tokenize_op(tokens, str_split, "*", TK_OP_MULT);
    tokenize_op(tokens, str_split, "/", TK_OP_DIV);
    tokenize_op(tokens, str_split, "%", TK_OP_MOD);

    tokenize_op(tokens, str_split, ">", TK_OP_GT);
    tokenize_op(tokens, str_split, "<", TK_OP_LT);
    tokenize_op(tokens, str_split, "!", TK_OP_NOT);

    tokenize_op(tokens, str_split, "|", TK_OP_BITOR);
    tokenize_op(tokens, str_split, "&", TK_OP_BITAND);
    tokenize_op(tokens, str_split, "~", TK_OP_COMPL);
    tokenize_op(tokens, str_split, "^", TK_OP_BITXOR);
    tokenize_op(tokens, str_split, "?", TK_OP_QST);
}

void tokenize_op(Tokens* tokens, StrVector *str_split, char* op, enum TokenType type) {
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
            Token* t = tokens_get(tokens, keyword_src_index);
            t->type = type;
            t->string_repr = op;
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
                if (!(isalnum(*str)) && *str != '_') { // Found end of identifier
                    int length = str - ident_start;
                    Token* t = tokens_get(tokens, src_pos-length);
                    t->type = TK_IDENT;
                    t->value.string = str_substr(ident_start, length);
                    t->string_repr = t->value.string;
                    t->requires_string_free = true;
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
            Token* t = tokens_get(tokens, src_pos-length);
            t->type = TK_IDENT;
            t->value.string = str_substr(ident_start, length);
            t->string_repr = t->value.string;
            t->requires_string_free = true;
            str_fill(ident_start, length, ' ');
            matching_ident = false;
        }
    }
}

// Tokenize int and float literals
// Floats: 1.3f, 1.3, .3
// Ints: 34312, 0xFAFA, don't do binary for now
void tokenize_values(Tokens *tokens, StrVector *str_split) {
    tokenize_ints(tokens, str_split);
    tokenize_floats(tokens, str_split);
}

void tokenize_ints(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    char* ident_start;
    bool matching = false;
    for (size_t i = 0; i < str_split->size; i++) {
        bool prev_is_whitespace = true;
        char* str = str_split->elems[i];
        while (*str != '\0') {
            if (!matching) {
                if (isdigit(*str) && prev_is_whitespace) { // Found start of int
                    matching = true;
                    ident_start = str;
                }
            }
            else {
                if (isalpha(*str) || *str == '_' || *str == '.') { // Invalid character in int
                    matching = false;
                    prev_is_whitespace = false;
                    continue;
                }
                else if (!isalnum(*str)) { // End of int
                    int length = str - ident_start;
                    Token* t = tokens_get(tokens, src_pos-length);
                    t->type = TK_LINT;
                    t->value.string = str_substr(ident_start, length);
                    t->string_repr = t->value.string;
                    t->requires_string_free = true;
                    str_fill(ident_start, length, ' ');
                    matching = false;
                }
            }
            if (!isalnum(*str) && *str != '_' && *str != '.') {
                prev_is_whitespace = true;
            }
            else {
                prev_is_whitespace = false;
            }
            str++;
            src_pos++;
        }
        if (matching) { // Found end of identifier, line ended
            int length = str - ident_start;
            Token* t = tokens_get(tokens, src_pos-length);
            t->type = TK_LINT;
            t->value.string = str_substr(ident_start, length);
            t->string_repr = t->value.string;
            t->requires_string_free = true;
            str_fill(ident_start, length, ' ');
            matching = false;
        }
    }
}

// Very similar to int, except there has to be a dot somewhere, either as first char or somewhere inside
void tokenize_floats(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    char* ident_start;
    bool matching = false;
    bool found_dot = false;
    bool found_digit = false;
    for (size_t i = 0; i < str_split->size; i++) {
        bool prev_is_whitespace = true;
        char* str = str_split->elems[i];
        while (*str != '\0') {
            if (!matching) {
                if ((isdigit(*str) || *str == '.') && prev_is_whitespace) { // Found start of int
                    matching = true;
                    ident_start = str;
                    if (*str == '.') {
                        found_dot = true;
                    }
                    else if (isdigit(*str)) {
                        found_digit = true;
                    }
                }
            }
            else {
                if (*str == '.') {
                    found_dot = true;
                }
                else if (isdigit(*str)) {
                    found_digit = true;
                }
                else if (isalpha(*str) || *str == '_' || (!isalpha(*str) && (!found_digit || !found_dot))) { // Invalid character in int
                    matching = false;
                    prev_is_whitespace = false;
                    found_dot = false;
                    continue;
                }
                else if (!isalnum(*str)) { // End of int
                    int length = str - ident_start;
                    Token* t = tokens_get(tokens, src_pos-length);
                    t->type = TK_LFLOAT;
                    t->value.string = str_substr(ident_start, length);
                    t->string_repr = t->value.string;
                    t->requires_string_free = true;
                    str_fill(ident_start, length, ' ');
                    matching = false;
                    found_dot = false;
                }
            }
            if (!isalnum(*str) && *str != '_' && *str != '.') {
                prev_is_whitespace = true;
            }
            else {
                prev_is_whitespace = false;
            }
            str++;
            src_pos++;
        }
        if (matching) { // Found end of identifier, line ended
            int length = str - ident_start;
            Token* t = tokens_get(tokens, src_pos-length);
            t->type = TK_LFLOAT;
            t->value.string = str_substr(ident_start, length);
            t->string_repr = t->value.string;
            t->requires_string_free = true;
            str_fill(ident_start, length, ' ');
            matching = false;
            found_dot = false;
        }
    }
}

void tokenize_delims(Tokens *tokens, StrVector *str_split) {
    int src_pos = 0;
    for (size_t i = 0; i < str_split->size; i++) {
        char* str = str_split->elems[i];
        while (*str != '\0') {
            Token* t = tokens_get(tokens, src_pos);
            switch (*str) {
                case '{':
                    *str = ' ';
                    t->type = TK_DL_OPENBRACE;
                    t->string_repr = "{";
                    break;
                case '}':
                    *str = ' ';
                    t->type = TK_DL_CLOSEBRACE;
                    t->string_repr = "}";
                    break;
                case '(':
                    *str = ' ';
                    t->type = TK_DL_OPENPAREN;
                    t->string_repr = "(";
                    break;
                case ')':
                    *str = ' ';
                    t->type = TK_DL_CLOSEPAREN;
                    t->string_repr = ")";
                    break;
                case '[':
                    *str = ' ';
                    t->type = TK_DL_OPENBRACKET;
                    t->string_repr = "[";
                    break;
                case ']':
                    *str = ' ';
                    t->type = TK_DL_CLOSEBRACKET;
                    t->string_repr = "]";
                    break;
                case '.':
                    *str = ' ';
                    t->type = TK_DL_DOT;
                    t->string_repr = ".";
                    break;
                case ',':
                    *str = ' ';
                    t->type = TK_DL_COMMA;
                    t->string_repr = ",";
                    break;
                case ';':
                    *str = ' ';
                    t->type = TK_DL_SEMICOLON;
                    t->string_repr = ";";
                    break;
                case ':':
                    *str = ' ';
                    t->type = TK_DL_COLON;
                    t->string_repr = ":";
                    break;
            }
            src_pos++;
            str++;
        }
    }
}

void tokens_print(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token* t = tokens_get(tokens, i);
        if (t->type == TK_NONE) {
            continue;
        }
        char* type_str = token_type_to_string(t->type);
        if (t->string_repr != 0) {
            printf("[T: %-18s free: %d   STR: \"%s\"]\n", type_str, t->requires_string_free, t->string_repr);
        }
        else {
            printf("[T: %s]\n", type_str);
        }
    }
}

Tokens tokens_copy(Tokens* tokens) {
    Tokens tokens_copy = *tokens;
    tokens_copy.elems = vec_copy(&tokens->elems);
    for (size_t i = 0; i < tokens->size; i++) {
        Token* token = tokens_get(tokens, i);
        Token* token_copy = tokens_get(&tokens_copy, i);
        if (token->requires_string_free) {
            token_copy->string_repr = str_copy(token->string_repr);
        }
    }
    return tokens_copy;
}

void tokens_pretty_print(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++)
    {
        Token* t = tokens_get(tokens, i);
        if (t->type == TK_DL_CLOSEBRACE) {
            printf("\n");
        }
        if (t->string_repr != 0) {
            printf("%s ", t->string_repr);
        }
        if (t->type == TK_DL_SEMICOLON || t->type == TK_DL_OPENBRACE) {
            printf("\n");
        }
    }
}

char* token_type_to_string(enum TokenType type) {
    static char *type_strings[] = {
        "TK_NONE", 
        "TK_IDENT", 
        "TK_TYPE", 
        "TK_COMMENT", 
        "TK_PREPROCESSOR",
        "TK_EOF",
        "TK_LINT", 
        "TK_LFLOAT", 
        "TK_LSTRING", 
        "TK_LCHAR",
        // Delimiters
        "TK_DL_SEMICOLON", 
        "TK_DL_COMMA", 
        "TK_DL_COLON", 
        "TK_DL_DOT", 
        "TK_DL_OPENPAREN", 
        "TK_DL_CLOSEPAREN", 
        "TK_DL_OPENBRACE", 
        "TK_DL_CLOSEBRACE", 
        "TK_DL_OPENBRACKET", 
        "TK_DL_CLOSEBRACKET",
        // Operations
        "TK_OP_PLUS", 
        "TK_OP_MINUS", 
        "TK_OP_MULT", 
        "TK_OP_DIV", 
        "TK_OP_MOD", 
        "TK_OP_RIGHTSHIFT", 
        "TK_OP_LEFTSHIFT", 
        "TK_OP_BITOR", 
        "TK_OP_BITAND",
        "TK_OP_BITXOR", 
        "TK_OP_COMPL", 
        "TK_OP_NOT", 
        "TK_OP_AND", 
        "TK_OP_OR", 
        "TK_OP_EQ", 
        "TK_OP_NEQ", 
        "TK_OP_GT", 
        "TK_OP_LT", 
        "TK_OP_LTE", 
        "TK_OP_GTE", 
        "TK_OP_QST",
        "TK_OP_INCR",
        "TK_OP_DECR",
        "TK_OP_ASSIGN", 
        "TK_OP_ASSIGN_ADD",
        "TK_OP_ASSIGN_SUB",
        "TK_OP_ASSIGN_MULT",
        "TK_OP_ASSIGN_DIV",
        "TK_OP_ASSIGN_MOD",
        "TK_OP_ASSIGN_RIGHTSHIFT",
        "TK_OP_ASSIGN_LEFTSHIFT",
        "TK_OP_ASSIGN_BITAND",
        "TK_OP_ASSIGN_BITOR",
        "TK_OP_ASSIGN_BITXOR",
        "TK_OP_SIZEOF",
        // Keywords
        "TK_KW_IF", 
        "TK_KW_ELSE", 
        "TK_KW_WHILE", 
        "TK_KW_DO", 
        "TK_KW_FOR", 
        "TK_KW_BREAK", 
        "TK_KW_CONTINUE", 
        "TK_KW_RETURN", 
        "TK_KW_SWITCH", 
        "TK_KW_CASE", 
        "TK_KW_DEFAULT", 
        "TK_KW_GOTO",
        "TK_KW_LABEL", 
        "TK_KW_TYPEDEF", 
        "TK_KW_INCLUDE", 
        "TK_KW_DEFINE", 
        "TK_KW_CONST", 
        "TK_KW_LONG", 
        "TK_KW_SHORT", 
        "TK_KW_SIGNED", 
        "TK_KW_UNSIGNED", 
        "TK_KW_STRUCT", 
        "TK_KW_UNION", 
        "TK_KW_INT", 
        "TK_KW_FLOAT", 
        "TK_KW_DOUBLE", 
        "TK_KW_CHAR", 
        "TK_KW_VOID"
    };
    return type_strings[type];
}