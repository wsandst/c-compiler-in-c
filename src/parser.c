#include "parser.h"

void ast_free(AST *ast) {
    ast_node_free(ast->program);
}

// Linked list used for freeing memory correctly
ASTNode *ast_node_end = NULL;

ASTNode *ast_node_new(ASTNodeType type, int count) {
    ASTNode* node = calloc(count, sizeof(ASTNode));
    node->type = type;

    // Memory managment, linked list used for freeing later
    if (ast_node_end != NULL) {
        ast_node_end->next_mem = node;
    }
    ast_node_end = node;

    return node;
}

void ast_node_free(ASTNode *ast_node) {
    // Keep the nodes in a linked list, to allow for easy freeing
    if (ast_node->next_mem != NULL) {
        ast_node_free(ast_node->next_mem);
    }
    free(ast_node);
}

Function *function_new(char *name) {
    Function* func = calloc(1, sizeof(Function));
    func->name = name;
    return func;
}

// This should be done recursively. Go deeper if necessary or backtrack. Always keep track of the position in the source
// We search for statements which always follow a predetermined pattern of tokens
// For example, int x = 5; is always TYPE IDENT ASSIGN. This is now an assignment
// if() = IF OPENPAREN STATEMENT CLOSEPAREN
// ident(); function call
// Statements are 
AST parse(Tokens *tokens) {
    // Find entry
    AST ast;
    int main_index = find_main_index(tokens);
    Token main_token = tokens->elems[main_index];
    Function *main_func = function_new(main_token.value.string);
    
    ASTNode *entry = ast_node_new(AST_FUNC, 1);
    entry->func = main_func;
    ast.program = entry;

    parse_decend(tokens, main_index, entry);

    return ast;
}

void parse_decend(Tokens *tokens, int token_i, ASTNode *node) {
    switch (node->type) {
        case AST_FUNC:
            parse_decend_func(tokens, token_i, node);
            break;
        default:
            break;
    }
}

// TODO: Add function hashmap
void parse_decend_func(Tokens *tokens, int token_i, ASTNode *node) {
    // Identify function name.
    char* func_name = tokens->elems[token_i].value.string;
    node->func = function_new(func_name);
    // Identify params
    token_i++;
    // This does not support const variables etc, pointers. Need to be expanded

    // We need to count how many args there are
    int i = token_i;
    int arg_count = 1;
    while (tokens->elems[i].value.delim != DL_CLOSEPAREN) {
        if (tokens->elems[i].type == TK_DELIMITER && tokens->elems[i].value.delim == DL_COMMA) {
            arg_count++;
        }
        i++;
    }

    ASTNode* arg_nodes = ast_node_new(AST_VAR, arg_count);

    while (tokens->elems[token_i].value.delim != DL_CLOSEPAREN) {
        token_i++;
        Variable arg;
        arg.type = token_type_to_var_type(tokens->elems[token_i + 0].type); // Variable Type
        arg.name = tokens->elems[token_i + 1].value.string; // Variable name 
        token_i += 2;
        arg_nodes->var = arg;
        arg_nodes++;
    }

    // We want to set up the arg types to the func

    node->func_args = arg_nodes;
}

int find_main_index(Tokens *tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        Token token = tokens->elems[i];
        if (token.type == TK_IDENT && strcmp(token.value.string, "main")) {
            return i;
        }
    }
    return -1;
}

VarTypeEnum token_type_to_var_type(enum KeywordType type) {
    switch (type) {
        case KW_INT:
            return TY_INT;
        case KW_FLOAT:
            return TY_FLOAT;
        case KW_DOUBLE:
            return TY_DOUBLE;
        case KW_CHAR:
            return TY_CHAR;
        default:
            return 0;
    }
}