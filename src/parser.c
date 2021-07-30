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
    if(ast_node->type == AST_LABEL || ast_node->type == AST_GOTO) {
        // These have allocated strings which are not freed anywhere else
        free(ast_node->literal);
    }
    free(ast_node);
}

void ast_node_swap(ASTNode* node1, ASTNode* node2) {
    ASTNode* tmp = ast_node_new(AST_EXPR, 1);
    memcpy(tmp, node1, sizeof(ASTNode));
    memcpy(node1, node2, sizeof(ASTNode));
    memcpy(node2, tmp, sizeof(ASTNode));
}

void ast_node_copy(ASTNode* node1, ASTNode* node2) {
    memcpy(node1, node2, sizeof(ASTNode));
}


void ast_free(AST* ast) {
    ast_node_free(ast_node_mem_start);
}

// Current token being parsed, global simplifies code a lot 
Token* parse_token;

AST parse(Tokens* tokens, SymbolTable* global_symbols) {
    parse_token = &tokens->elems[0];
    // Setup initial AST
    AST ast;
    ASTNode *program_node = ast_node_new(AST_PROGRAM, 1);
    ast.program = program_node;
    program_node->body = ast_node_new(AST_NONE, 1);

    // Start parsing
    parse_program(program_node->body, global_symbols);

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

// Accept a unary operator with two tokens (++ and --)
bool accept_unop_two_token_type() {
    if (accept(TK_OP_MINUS)) {
        if (accept(TK_OP_MINUS)) {
            (parse_token-1)->type = TK_OP_DECR;
            return true;
        }
        token_go_back(1);
    }
    else if (accept(TK_OP_PLUS)) {
        if (accept(TK_OP_PLUS)) {
            (parse_token-1)->type = TK_OP_INCR;
            return true;
        }
        token_go_back(1);
    }
    return false;
}

bool accept_unop() {
    return (accept_unop_two_token_type() || accept(TK_OP_MINUS) || accept(TK_OP_NOT) || accept(TK_OP_COMPL));
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
    expect(TK_IDENT);
    if (accept(TK_DL_OPENPAREN)) { // Function
        token_go_back(3);
        parse_func(node, symbols);
    }
    else if (accept(TK_DL_SEMICOLON) || accept(TK_OP_ASSIGN)) { // Global declaration or assignment
        token_go_back(2);
        parse_statement(node, symbols);
    }
    node->next = ast_node_new(AST_NONE, 1);
    parse_program(node->next, symbols);
}

void parse_func(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_FUNC;
    Function func;
    expect_var_type();
    enum VarTypeEnum type = token_type_to_var_type(prev_token().type);
    expect(TK_IDENT);
    char* ident = prev_token().value.string;
    func.name = ident;
    func.return_type = type;

    // Create new scope for function
    SymbolTable* func_symbols = symbol_table_create_child(symbols, 0);

    expect(TK_DL_OPENPAREN);
    // Parse function arguments
    while (!accept(TK_DL_CLOSEPAREN)) { // Add argument variables to symbol map
        Variable var; 
        expect_var_type();
        var.type = token_type_to_var_type(prev_token().type);
        expect(TK_IDENT);
        var.name = prev_token().string_repr;
        var.size = 8;
        accept(TK_DL_COMMA);
        symbol_table_insert_var(func_symbols, var);
    }
    // We can directly take the variables pointer, as nothing else will be added
    // in this scope
    func.params = func_symbols->vars;
    func.param_count = func_symbols->var_count;
    node->func = symbol_table_insert_func(symbols, func);

    expect(TK_DL_OPENBRACE);
    node->body = ast_node_new(AST_NONE, 1);
    parse_scope(node->body, func_symbols);
    node->next = ast_node_new(AST_END, 1);
}

void parse_single_statement(ASTNode* node, SymbolTable* symbols) {
    if (accept(TK_COMMENT)) { // Comment, do nothing, move on to next statement
        parse_single_statement(node, symbols);
        return;
    }
    else if (accept_var_type()) { // Variable declaration
        // Declaration is just connected to the symbol table,
        // no actual node needed

        // Add to symbol table
        Variable var;
        var.type = token_type_to_var_type(prev_token().type);
        expect(TK_IDENT);
        char* ident = prev_token().value.string;
        var.name = ident;
        var.size = 8; // 64 bit
        symbol_table_insert_var(symbols, var);

        if (accept(TK_OP_ASSIGN)) { // Def and assignment
            // Treat this as an expresison
            token_go_back(2); // Go back to ident token
            node->type = AST_EXPR;
            node->assign = ast_node_new(AST_EXPR, 1);
            node->top_level_expr = true;
            parse_expression(node, symbols);
        }
        else {
            expect(TK_DL_SEMICOLON);
            // We can reuse this node, assignment is just virtual
            parse_single_statement(node, symbols);
            return;
        }
    }
    else if (accept(TK_IDENT)) { // I need to handle literals here too
        if (accept(TK_DL_COLON)) { // Goto label
            node->type = AST_LABEL;
            token_go_back(1);
            // We need to add a prefix here so the goto labels
            // don't conflict with our own internal labels
            node->literal = str_add("G", prev_token().string_repr);
            expect(TK_DL_COLON);
        }
        else {
            // Treat as expression (probably assignment of some sort)
            token_go_back(1);
            node->top_level_expr = true;
            parse_expression(node, symbols);
        }
    }
    else if (accept(TK_KW_IF)) { // If conditional
        parse_if(node, symbols);
    }
    else if (accept(TK_KW_WHILE)) { // While loop
        parse_while_loop(node, symbols);
    }
    else if (accept(TK_KW_DO)) { // Do while loop
        parse_do_while_loop(node, symbols);
    }
    else if (accept(TK_KW_FOR)) { // For loop
        parse_for_loop(node, symbols);
    }
    else if (accept(TK_KW_BREAK)) { // Break loop/switch
        node->type = AST_BREAK;
    }
    else if (accept(TK_KW_CONTINUE)) { // Continue loop
        node->type = AST_CONTINUE;
    }
    else if (accept(TK_KW_SWITCH)) {
        parse_switch(node, symbols);
    }
    else if (accept(TK_KW_RETURN)) { // Return statements
        node->type = AST_RETURN;
        node->ret = ast_node_new(AST_EXPR, 1);
        parse_expression(node->ret, symbols);
    }
    else if (accept(TK_KW_CASE)) { // Case statements
        parse_case(node, symbols);
    }
    else if (accept(TK_KW_DEFAULT)) { // Default case
        parse_default_case(node, symbols);
    }
    else if (accept(TK_KW_GOTO)) {
        node->type = AST_GOTO;
        expect(TK_IDENT);
        // We don't do any checks if the label exists here, would
        // require two passes. Let the assembler handle it
        node->literal = str_add("G", prev_token().string_repr);
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
    node->next = ast_node_new(AST_END, 1);
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
            token_go_back(1);
            parse_func_call(node, symbols);
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
        node->op_type = token_type_to_pre_uop_type(prev_token().type);
        node->rhs = ast_node_new(AST_EXPR, 1);
        parse_expression(node->rhs, symbols);
        return;
    }
    else if (accept(TK_DL_CLOSEPAREN)) {
        node->type = AST_NONE;
        return; // Atom end 
    }
    else {
        parse_error("Invalid expression");
    }
    if (accept(TK_DL_CLOSEPAREN)) {
        symbols->cur_stack_offset -= 8;
        return; // Atom end 
    }
    if (accept_unop_two_token_type()) { // Accept postfix unary operators
        ASTNode* rhs = ast_node_new(AST_EXPR, 1);
        ast_node_copy(rhs, node);
        node->rhs = rhs;
        node->expr_type = EXPR_UNOP;
        node->op_type = token_type_to_post_uop_type(prev_token().type);
        //parse_expression(node->rhs, symbols); //x++ x + 2
    }
    if (accept_binop()) { 
        // Binary op next up, we need to change this node to binop
        // and set the previous values to the lhs node

        // Copy this node to node->lhs
        ASTNode* lhs = ast_node_new(AST_EXPR, 1);
        ast_node_copy(lhs, node);
        node->lhs = lhs;
        node->rhs = ast_node_new(AST_EXPR, 1);
        node->expr_type = EXPR_BINOP;
        node->op_type = token_type_to_bop_type(prev_token().type);

        // Every operation gets a result space, which is just
        // cur_stack_offset + 8. When we've finished using it, revert cur_stack_offset.
        symbols->cur_stack_offset += 8; 
        node->scratch_stack_offset = symbols->cur_stack_offset;

        parse_expression(node->rhs, symbols);
        symbols->cur_stack_offset -= 8;
    }
    else if (accept(TK_DL_SEMICOLON) || accept(TK_DL_COMMA) || accept(TK_DL_OPENBRACE) || accept(TK_DL_CLOSEPAREN)) {
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

void parse_func_call(ASTNode* node, SymbolTable* symbols) {
    node->expr_type = EXPR_FUNC_CALL;
    char* ident = prev_token().string_repr;
    expect(TK_DL_OPENPAREN);
    node->func = symbol_table_lookup_func(symbols, ident);
    node->args = ast_node_new(AST_EXPR, 1);
    ASTNode* arg_node = node->args;
    while (!(accept(TK_DL_CLOSEPAREN) || prev_token().type == TK_DL_CLOSEPAREN)) { // Go through argument expressions
        symbols->cur_stack_offset += 8;
        parse_expression(arg_node, symbols);
        symbols->cur_stack_offset -= 8;
        arg_node->next = ast_node_new(AST_END, 1);
        arg_node = arg_node->next;
    }
    symbols->cur_stack_offset += 8;
}

void parse_if(ASTNode* node, SymbolTable* symbols) {
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

// Parse a while loop
void parse_while_loop(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_LOOP;
    node->cond = ast_node_new(AST_EXPR, 1);
    expect(TK_DL_OPENPAREN);
    parse_expression(node->cond, symbols);
    symbols->cur_stack_offset += 8;
    node->then = ast_node_new(AST_BLOCK, 1);
    parse_single_statement(node->then, symbols);
    node->then->next = ast_node_new(AST_END, 1);
}

// Parse a do while loop
void parse_do_while_loop(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_DO_LOOP;
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

// Parse a for loop
void parse_for_loop(ASTNode* node, SymbolTable* symbols) {
    // Same construction as while loop, but we need to insert a few additional statements
    // for(int i = 0; i < 10, i++) becomes
    // int i = 0; while(i < 10) { i++; ... }

    // We need a new scope for the for variable declaration
    node->type = AST_BLOCK;
    node->body = ast_node_new(AST_STMT, 1);
    SymbolTable* scope_symbols = symbol_table_create_child(symbols, symbols->cur_stack_offset);
    node->next = ast_node_new(AST_STMT, 1);

    // Set the first statement to the for init statement
    expect(TK_DL_OPENPAREN);
    parse_single_statement(node->body, scope_symbols);

    // Now we can treat this like a while loop almost
    node->body->next = ast_node_new(AST_LOOP, 1);
    ASTNode* loop_node = node->body->next;
    loop_node->next = ast_node_new(AST_NONE, 1);

    // Getting the condition
    loop_node->cond = ast_node_new(AST_EXPR, 1);
    scope_symbols->cur_stack_offset += 8;
    parse_expression(loop_node->cond, scope_symbols);

    // Getting the increment expression
    loop_node->incr = ast_node_new(AST_EXPR, 1);
    scope_symbols->cur_stack_offset += 8;
    parse_expression(loop_node->incr, scope_symbols);
    loop_node->incr->next = ast_node_new(AST_END, 1);

    // Parsing the for-loop body
    loop_node->then = ast_node_new(AST_STMT, 1);
    parse_single_statement(loop_node->then, scope_symbols);

    // We need to insert the increment operation last
    loop_node->then->next = ast_node_new(AST_END, 1);
}

// Parse a switch statement
void parse_switch(ASTNode* node, SymbolTable* symbols) {
    // Create a new switch scope
    node->type = AST_SWITCH;
    SymbolTable* switch_symbols = symbol_table_create_child(symbols, symbols->cur_stack_offset);
    switch_symbols->is_switch_scope = true;
    switch_symbols->label_prefix++;

    expect(TK_DL_OPENPAREN);
    // Get the switch value
    node->cond = ast_node_new(AST_EXPR, 1);
    switch_symbols->cur_stack_offset += 8;
    parse_expression(node->cond, switch_symbols);
    // Now we can parse the contents
    node->body = ast_node_new(AST_STMT, 1);
    node->body->next = ast_node_new(AST_NONE, 1);
    parse_single_statement(node->body, switch_symbols);
    // We now need to grab the case labels and store them in the AST Node
    node->switch_cases = symbol_table_lookup_switch_case_labels(switch_symbols);
    node->next = ast_node_new(AST_NONE, 1);
}

void parse_case(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_CASE;
    expect(TK_LINT);
    ValueLabel label;
    label.value = prev_token().string_repr;
    label.is_default_case = false;

    label = symbol_table_insert_label(symbols, label);
    node->label = label;
    expect(TK_DL_COLON);
}

void parse_default_case(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_CASE;
    expect(TK_DL_COLON);
    ValueLabel label;
    label.is_default_case = true;
    label = symbol_table_insert_label(symbols, label);
    node->label = label;
}

void parse_error(char* error_message) {
    fprintf(stderr, "Parse error: %s (token: \"%s\")\n", error_message, token_type_to_string(parse_token->type));
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

OpType token_type_to_pre_uop_type(enum TokenType type) {
    switch (type) {
        case TK_OP_MINUS:
            return UOP_NEG;
        case TK_OP_NOT:
            return UOP_NOT;
        case TK_OP_COMPL:
            return UOP_COMPL;
        case TK_OP_INCR: 
            return UOP_PRE_INCR;
        case TK_OP_DECR:
            return UOP_PRE_DECR;
        default:
            parse_error("Unsupported prefix unary operation encountered while parsing");
            return 0;
    }
}
OpType token_type_to_post_uop_type(enum TokenType type) {
    switch (type) {
        case TK_OP_INCR: 
            return UOP_POST_INCR;
        case TK_OP_DECR:
            return UOP_POST_DECR;
        default:
            parse_error("Unsupported postfix unary operation encountered while parsing");
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