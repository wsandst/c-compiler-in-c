#include "parser.h"

// Todo:
// Convert integers to actual int values (do this in code gen)
// Code generation


// Linked list used for freeing memory correctly
ASTNode *ast_node_mem_end = NULL;
ASTNode *ast_node_mem_start;

ASTNode *ast_node_new(ASTNodeType type, int count) {
    ASTNode* node = calloc(count, sizeof(ASTNode));
    node->type = type;

    // Memory managment, linked list used for freeing later
    if (ast_node_mem_end != NULL) {
        ast_node_mem_end->next_mem = node;
    }
    else {
        ast_node_mem_start = node;
    }
    ast_node_mem_end = node;

    return node;
}

void ast_node_free(ASTNode *ast_node) {
    // Keep the nodes in a linked list, to allow for easy freeing
    if (ast_node->next_mem != NULL) {
        ast_node_free(ast_node->next_mem);
    }
    free(ast_node);
}

// Linked list used for freeing memory correctly
Function *function_mem_end = NULL;
Function *function_mem_start;

Function *function_new(char *name) {
    Function* func = calloc(1, sizeof(Function));
    func->name = name;

    // Memory managment, linked list used for freeing later
    if (function_mem_end != NULL) {
        function_mem_end->next_mem = func;
    }
    else {
        function_mem_start = func;
    }
    function_mem_end = func;

    return func;
}

void function_free(Function* func) {
    if (func->next_mem != NULL) {
        function_free(func->next_mem);
    }
    free(func);
}

void ast_free(AST* ast) {
    ast_node_free(ast_node_mem_start);
    function_free(function_mem_start);
}

// Current token being parsed, global simplifies code a lot 
Token* parse_token;

AST parse(Tokens* tokens) {
    // Find entry
    AST ast;
    int main_index = find_main_index(tokens);
    //Function *main_func = function_new(main_token.value.string);
    ASTNode *main_node = ast_node_new(AST_NONE, 1);
    ast.program = main_node;

    SymbolTable* global_symbols = symbol_table_new();

    parse_token = &tokens->elems[main_index];

    SymbolTable* main_symbols = symbol_table_create_child(global_symbols, 0);
    parse_func(main_node, main_symbols);

    symbol_table_free(global_symbols);

    return ast;
}
Token prev_token() {
    return *(parse_token - 1);
}

void expect(enum TokenType type) {
    if (parse_token->type != type) {
        parse_error_unexpected_symbol(type, parse_token->type);
    }
    parse_token++;
}

void expect_var_type() {
    if (accept(TK_KW_INT) || accept(TK_KW_FLOAT) || accept(TK_KW_DOUBLE) || accept(TK_KW_CHAR)) {
        return;
    }
    else {
        parse_error("Unexpected symbol");
    }
}

bool accept(enum TokenType type) {
    if (parse_token->type == type) {
        parse_token++;
        return true;
    }
    else {
        return false;
    }
}

bool accept_var_type() {
    return (accept(TK_KW_INT) || accept(TK_KW_FLOAT) || accept(TK_KW_DOUBLE) || accept(TK_KW_CHAR));
}

void parse_func(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_FUNC;
    expect_var_type();
    enum VarTypeEnum return_type = token_type_to_var_type(prev_token().type);
    expect(TK_IDENT);
    char* func_name = prev_token().value.string;
    Function *func = function_new(func_name);
    func->return_type = return_type;
    node->func = func;
    expect(TK_DL_OPENPAREN);
    expect(TK_DL_CLOSEPAREN);
    expect(TK_DL_OPENBRACE);
    ASTNode* stmt = ast_node_new(AST_NONE, 1);
    node->body = stmt;
    parse_statement(stmt, symbols);
}

void parse_statement(ASTNode* node, SymbolTable* symbols) {
    if (accept_var_type()) { // Variable declaration
        Variable var;
        node->type = AST_VAR_DEC;
        var.type = token_type_to_var_type(prev_token().type);
        expect(TK_IDENT);
        char* ident = prev_token().value.string;
        var.name = ident;
        var.size = 8; // 64 bit
        node->var = symbol_table_insert_var(symbols, var);

        if (accept(TK_OP_ASSIGN)) { // Def and assignment
            node->type = AST_ASSIGN;
            node->assign = ast_node_new(AST_EXPR, 1);
            parse_expression(node->assign, symbols);
        }
        else {
            expect(TK_DL_SEMICOLON);
        }
    }
    else if (accept(TK_IDENT)) {
        char* ident = prev_token().value.string;
        Variable var = symbol_table_lookup_var(symbols, ident);
        node->var = var;
        expect(TK_OP_ASSIGN);
        node->type = AST_ASSIGN;
        node->assign = ast_node_new(AST_EXPR, 1);
        parse_expression(node->assign, symbols);
    }
    else if (accept(TK_KW_RETURN)) {
        node->type = AST_RETURN;
        node->ret = ast_node_new(AST_EXPR, 1);
        parse_expression(node->ret, symbols);
    }
    else if (accept(TK_DL_CLOSEBRACE)) {
        // Scope or function end
        node->type = AST_NONE;
        return;
    }
    else if (accept(TK_DL_OPENBRACE)) {
        // Scope start
        parse_scope(node, symbols); 
    }
    else if (accept(TK_DL_CLOSEBRACE)) { // End of scope
        return;
    }
    node->next = ast_node_new(AST_STMT, 1);
    parse_statement(node->next, symbols);
}

void parse_scope(ASTNode* node, SymbolTable* symbols) {
    // Make a new child symbol table for this scope
    node->type = AST_BLOCK;
    node->body = ast_node_new(AST_STMT, 1);
    SymbolTable* scope_symbols = symbol_table_create_child(symbols, symbols->cur_stack_offset);
    parse_statement(node->body, scope_symbols);
}

void parse_expression(ASTNode* node, SymbolTable* symbols) {
    // Do Shunting-yard algorithm
    // We only handle integer constants currently
    if (accept(TK_LINT)) {
        node->type = AST_NUM;
        node->literal = prev_token().string_repr;
    }
    // Variable
    else if (accept(TK_IDENT)) {
        node->type = AST_VAR;
        node->var = symbol_table_lookup_var(symbols, prev_token().string_repr);
    }
    else {
        parse_error("Invalid expression");
    }
    if (accept(TK_DL_SEMICOLON) || accept(TK_DL_COMMA)) {
        return; // Expression end
    }
    else {
        parse_error("Invalid expression");
    }
}

void parse_error(char* error_message) {
    fprintf(stderr, "Parse error: %s\n", error_message);
    // We are not manually freeing the memory here, 
    // but as the program is exiting it is fine
    exit(1); 
}

void parse_error_unexpected_symbol(enum TokenType expected, enum TokenType recieved) {
    char buff[256];
    char* expected_str = token_type_to_string(expected);
    char* recieved_str = token_type_to_string(recieved);
    snprintf(buff, 255, "Expected symbol '%s', found symbol '%s'", expected_str, recieved_str);
    parse_error(buff);
}


int find_main_index(Tokens* tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        Token token = tokens->elems[i];
        if (token.type == TK_IDENT && strcmp(token.value.string, "main") == 0) {
            return i-1;
        }
    }
    return -1;
}

VarTypeEnum token_type_to_var_type(enum TokenType type) {
    switch (type) {
        case TK_KW_INT:
            return TY_INT;
        case TK_KW_FLOAT:
            return TY_FLOAT;
        case TK_KW_DOUBLE:
            return TY_DOUBLE;
        case TK_KW_CHAR:
            return TY_CHAR;
        default:
            return 0;
    }
}