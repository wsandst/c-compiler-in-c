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
    ASTNode *program_node = ast_node_new(AST_PROGRAM, 1);
    ast.program = program_node;
    program_node->body = ast_node_new(AST_NONE, 1);

    // Create global symbol table
    SymbolTable* global_symbols = symbol_table_new();

    // Start parsing
    parse_program(program_node->body, global_symbols);

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

bool accept_unop() {
    return (accept(TK_OP_MINUS) || accept(TK_OP_NOT) || accept(TK_OP_COMPL));
}

bool accept_binop() {
    return (
        accept(TK_OP_PLUS) || accept(TK_OP_MINUS) || accept(TK_OP_MULT) || accept(TK_OP_DIV) || 
        accept(TK_OP_EQ) || accept(TK_OP_NEQ) || accept(TK_OP_LT) || accept(TK_OP_LTE) || 
        accept(TK_OP_GT) || accept(TK_OP_GTE) || accept(TK_OP_MOD) || accept(TK_OP_AND) || 
        accept(TK_OP_OR) || accept(TK_OP_ASSIGN)
        );
}

void parse_program(ASTNode* node, SymbolTable* symbols) {
    // Either a function or a global
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
        // Create new scope for function
        SymbolTable* func_symbols = symbol_table_create_child(symbols, 0);
        parse_func(node, func_symbols);
    }
    else if (accept(TK_DL_SEMICOLON) || accept(TK_OP_ASSIGN)) { // Global declaration or assignment
        token_go_back(2);
        parse_statement(node, symbols);
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

void parse_single_statement(ASTNode* node, SymbolTable* symbols) {
    if (accept(TK_COMMENT)) { // Comment, do nothing, move on to next statement
        parse_single_statement(node, symbols);
        return;
    }
    else if (accept_var_type()) { // Variable declaration
        // Declaration is just connected to the symbol table,
        // no actual node needed
        Variable var;
        var.type = token_type_to_var_type(prev_token().type);
        expect(TK_IDENT);
        char* ident = prev_token().value.string;
        var.name = ident;
        var.size = 8; // 64 bit
        symbol_table_insert_var(symbols, var);

        if (accept(TK_OP_ASSIGN)) { // Def and assignment
            // Treat this as an expresison
            token_go_back(2); // Go back to ident
            node->type = AST_EXPR;
            node->assign = ast_node_new(AST_EXPR, 1);
            node->top_level_expr = true;
            parse_expression(node, symbols);
        }
        else {
            expect(TK_DL_SEMICOLON);
            // We can reuse this node
            parse_single_statement(node, symbols);
            return;
        }
    }
    else if (accept(TK_IDENT)) {
        // Treat as expression (probably assignment of some sort)
        token_go_back(1);
        node->top_level_expr = true;
        parse_expression(node, symbols);
    }
    else if (accept(TK_KW_IF)) { // If statements
        node->type = AST_IF;
        node->cond = ast_node_new(AST_EXPR, 1);
        expect(TK_DL_OPENPAREN);
        parse_expression(node->cond, symbols);
        symbols->cur_stack_offset += 8;
        node->then = ast_node_new(AST_BLOCK, 1);
        parse_single_statement(node->then, symbols);
        node->then->next = ast_node_new(AST_END, 1);
        // Check if the if has an attached else
        if (accept(TK_KW_ELSE)) {
            node->els = ast_node_new(AST_STMT, 1);
            parse_single_statement(node->els, symbols);
            node->els->next = ast_node_new(AST_END, 1);
        }
    }
    else if (accept(TK_KW_WHILE)) { // While statement
        node->type = AST_WHILE;
        node->cond = ast_node_new(AST_EXPR, 1);
        expect(TK_DL_OPENPAREN);
        parse_expression(node->cond, symbols);
        symbols->cur_stack_offset += 8;
        node->then = ast_node_new(AST_BLOCK, 1);
        parse_single_statement(node->then, symbols);
        node->then->next = ast_node_new(AST_END, 1);
    }
    else if (accept(TK_KW_DO)) { // Do while statement
        node->type = AST_DO_WHILE;
        // Parse do while body
        node->then = ast_node_new(AST_BLOCK, 1);
        parse_single_statement(node->then, symbols);
        node->then->next = ast_node_new(AST_END, 1);
        // Parse while condition at end
        expect(TK_KW_WHILE);
        expect(TK_DL_OPENPAREN);
        symbols->cur_stack_offset += 8;
        node->cond = ast_node_new(AST_EXPR, 1);
        parse_expression(node->cond, symbols);
    }
    else if (accept(TK_KW_RETURN)) { // Return statements
        node->type = AST_RETURN;
        node->ret = ast_node_new(AST_EXPR, 1);
        parse_expression(node->ret, symbols);
    }
    else if (accept(TK_DL_OPENBRACE)) { // Scope begin
        parse_scope(node, symbols);
    }
    else if (accept(TK_DL_CLOSEBRACE)) { // Scope end
        token_go_back(1); // Return to parse_statement
        return;
    }
    else if (accept(TK_DL_SEMICOLON)) {
        // Null statement
        node->type = AST_NULL_STMT;
    }
    else {
        parse_error("Invalid statement");
    }
}

void parse_statement(ASTNode* node, SymbolTable* symbols) {
    if (accept(TK_DL_CLOSEBRACE)) {
        // Scope or function end
        node->type = AST_NONE;
        return;
    }
    parse_single_statement(node, symbols);
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
    node->type = AST_EXPR;

    // Is this an atom? (non-operation)
    if (accept(TK_LINT)) { // Literal
        node->expr_type = EXPR_LITERAL;
        node->literal = prev_token().string_repr;
    }
    else if (accept(TK_IDENT)) { // Variable or function call
        char* ident = prev_token().string_repr;
        if (accept(TK_DL_OPENPAREN)) { // Function call
            node->expr_type = EXPR_FUNC_CALL;
            node->func = symbol_table_lookup_func(symbols, ident);
            expect(TK_DL_CLOSEPAREN);
        }  
        else { // Variable
            node->expr_type = EXPR_VAR;
            node->var = symbol_table_lookup_var(symbols, ident);
        }
    }
    else if (accept(TK_DL_OPENPAREN)) {
        symbols->cur_stack_offset += 8;
        parse_expression(node, symbols);
    }
    // Non-atom
    else if (accept_unop()) { // Unary operation
        node->expr_type = EXPR_UNOP;
        node->op_type = token_type_to_uop_type(prev_token().type);
        node->rhs = ast_node_new(AST_EXPR, 1);
        parse_expression(node->rhs, symbols);
        return;
    }
    else {
        parse_error("Invalid expression");
    }
    if (accept(TK_DL_CLOSEPAREN)) {
        symbols->cur_stack_offset -= 8;
        return; // Atom end 
    }
    if (accept_binop()) { 
        // Binary op next up, we need to change this node to binop
        // and set the previous values to the lhs node

        // Copy this node to node->lhs
        ASTNode* new_lhs = ast_node_new(AST_EXPR, 1);
        new_lhs->lhs = node->lhs;
        node->lhs = new_lhs;
        node->lhs->rhs = node->rhs;
        node->rhs = ast_node_new(AST_EXPR, 1);
        node->lhs->expr_type = node->expr_type;
        node->lhs->op_type = node->op_type;
        node->lhs->func = node->func;
        node->lhs->var = node->var;
        node->lhs->literal = node->literal;
        node->lhs->scratch_stack_offset = node->scratch_stack_offset;
        node->expr_type = EXPR_BINOP;
        node->op_type = token_type_to_bop_type(prev_token().type);

        // Every operation gets a result space, which is just
        // cur_stack_offset + 8. When we've finished using it, revert cur_stack_offset.
        symbols->cur_stack_offset += 8; 
        node->scratch_stack_offset = symbols->cur_stack_offset;

        parse_expression(node->rhs, symbols);
        symbols->cur_stack_offset -= 8;
    }
    else if (accept(TK_DL_SEMICOLON) || accept(TK_DL_COMMA) || accept(TK_DL_OPENBRACE)) {
        return; 
    }
    else if (accept(TK_DL_CLOSEBRACE)) {
        token_go_back(1);
        return;
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

OpType token_type_to_uop_type(enum TokenType type) {
    switch (type) {
        case TK_OP_MINUS:
            return UOP_NEG;
        case TK_OP_NOT:
            return UOP_NOT;
        case TK_OP_COMPL:
            return UOP_COMPL;
        default:
            parse_error("Unsupported unary operation encountered while parsing");
            return 0;
    }
}

OpType token_type_to_bop_type(enum TokenType type) {
    switch (type) {
        case TK_OP_PLUS:
            return BOP_ADD;
        case TK_OP_MINUS:
            return BOP_SUB;
        case TK_OP_MULT:
            return BOP_MUL;
        case TK_OP_DIV:
            return BOP_DIV;
        case TK_OP_MOD:
            return BOP_MOD;
        case TK_OP_EQ:
            return BOP_EQ;
        case TK_OP_NEQ:
            return BOP_NEQ;
        case TK_OP_LT:
            return BOP_LT;
        case TK_OP_LTE:
            return BOP_LTE;
        case TK_OP_GT:
            return BOP_GT;
        case TK_OP_GTE:
            return BOP_GTE;
        case TK_OP_AND:
            return BOP_AND;
        case TK_OP_OR:
            return BOP_OR;
        case TK_OP_ASSIGN:
            return BOP_ASSIGN;
        default:
            parse_error("Unsupported binary operation encountered while parsing");
            return 0;
    }
}