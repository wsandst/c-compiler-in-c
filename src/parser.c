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


void ast_free(AST* ast) {
    ast_node_free(ast_node_mem_start);
}

// Current token being parsed, global simplifies code a lot 
Token* parse_token;

AST parse(Tokens* tokens) {
    parse_token = &tokens->elems[0];
    // Setup initial AST
    AST ast;
    ASTNode *program_node = ast_node_new(AST_END, 1);
    program_node->type = AST_PROGRAM;
    ast.program = program_node;

    // Create global symbol table
    SymbolTable* global_symbols = symbol_table_new();


    // Start parsing
    parse_program(program_node, global_symbols);

    // Free memory
    symbol_table_free(global_symbols);

    return ast;
}
Token prev_token() {
    return *(parse_token - 1);
}

void token_go_back(int steps) {
    parse_token = parse_token - steps;
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

void parse_program(ASTNode* node, SymbolTable* symbols) {
    // Either a function or a global, add preprocessor stuff later
    // These should probably be handled as normal statements, not
    // hardcoded up in the program. Will simplify variable handling
    if (accept(TK_EOF)) { // Reached end of program
        node->type = AST_END;
        return;
    }
    if (accept(TK_COMMENT) || accept(TK_PREPROCESSOR)) { // Skip
        parse_program(node, symbols);
        return;
    }
    // Must either be a function or a global variable
    expect_var_type();
    enum VarTypeEnum type = token_type_to_var_type(prev_token().type);
    expect(TK_IDENT);
    char* ident = prev_token().value.string;
    if (accept(TK_DL_OPENPAREN)) { // Function
        node->type = AST_FUNC;
        Function func;
        func.name = ident;
        func.return_type = type;
        func = symbol_table_insert_func(symbols, func);
        node->func = func;
        parse_func(node, symbols);
    }
    else if (accept(TK_DL_SEMICOLON) || accept(TK_OP_ASSIGN)) { // Global declaration or assignment
        token_go_back(2);
        parse_statement(symbols, node);
    }
    node->next = ast_node_new(AST_NONE, 1);
    parse_program(node->next, symbols);
}

void parse_func(ASTNode* node, SymbolTable* symbols) {
    //Function *func = function_new(func_name);
    //func->return_type = return_type;
    //node->func = func;
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