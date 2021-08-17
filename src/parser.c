#include "parser.h"

// Todo:
// Convert integers to actual int values (do this in code gen)
// Code generation


// Linked list used for freeing memory correctly
static ASTNode *ast_node_mem_end = NULL;
static ASTNode *ast_node_mem_start;

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
static Token* parse_token;
static VarType latest_parsed_var_type;
static Function latest_func;

AST parse(Tokens* tokens, SymbolTable* global_symbols) {
    parse_token = tokens_get(tokens, 0);
    // Setup initial AST
    AST ast;
    ASTNode *program_node = ast_node_new(AST_PROGRAM, 1);
    ast.program = program_node;
    program_node->body = ast_node_new(AST_END, 1);

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
    if (!accept(type)) {
        parse_error_unexpected_symbol(type, parse_token->type);
    }
}

void expect_type() {
    if (accept_type()) {
        return;
    }
    else {
        parse_error("Unexpected symbol");
    }
}

bool accept(TokenType type) {
    if (parse_token->type == type) {
        //printf("%s (%s)\n", token_type_to_string(parse_token->type), parse_token->string_repr);
        parse_token++;
        return true;
    }
    else {
        return false;
    }
}

// Accept any token within this range
bool accept_range(TokenType from_token, TokenType to_token) {
    if (parse_token->type >= from_token && parse_token->type <= to_token) {
        //printf("%s\n", token_type_to_string(parse_token->type));
        parse_token++;
        return true;
    }
    return false;
}

bool accept_unop() {
    return (accept(TK_OP_MINUS) || accept(TK_OP_NOT) || accept(TK_OP_COMPL) 
        || accept(TK_OP_INCR) || accept(TK_OP_DECR) || accept(TK_OP_SIZEOF)
        || accept(TK_OP_BITAND) || accept(TK_OP_MULT));
}

bool accept_post_unop() {
    return (accept(TK_OP_INCR) || accept(TK_OP_DECR));
}

bool accept_binop() {
    return accept_range(TK_OP_PLUS, TK_OP_ASSIGN_BITXOR);
}

// Accept variable/function type: float, double, char, short, int, long
bool accept_type() {
    accept(TK_KW_CONST);
    latest_parsed_var_type.is_static = false;
    latest_parsed_var_type.is_extern = false;
    if (accept(TK_KW_EXTERN)) {
        latest_parsed_var_type.is_extern = true;
    };
    if (accept(TK_KW_STATIC)) {
        latest_parsed_var_type.is_static = true;
    }
    latest_parsed_var_type.ptr_level = 0;
    if (accept(TK_KW_UNSIGNED)) {
        latest_parsed_var_type.is_unsigned = true;
    }
    else {
        accept(TK_KW_SIGNED);
        latest_parsed_var_type.is_unsigned = false;
    }
    if (accept(TK_KW_CHAR)) {
        latest_parsed_var_type.type = TY_INT;
        latest_parsed_var_type.bytes = 1;
    }
    else if (accept(TK_KW_SHORT)) {
        latest_parsed_var_type.type = TY_INT;
        latest_parsed_var_type.bytes = 2;
        accept(TK_KW_INT);
    }
    else if (accept(TK_KW_INT)) {
        latest_parsed_var_type.type = TY_INT;
        latest_parsed_var_type.bytes = 4;
    }
    else if (accept(TK_KW_LONG)) {
        latest_parsed_var_type.type = TY_INT;
        latest_parsed_var_type.bytes = 8;
        if (accept(TK_KW_DOUBLE)) {
            latest_parsed_var_type.type = TY_FLOAT;
            latest_parsed_var_type.bytes = 8;
        }
        else {
            accept(TK_KW_LONG);
            accept(TK_KW_INT);
        }
    }
    else if (accept(TK_KW_FLOAT)) {
        latest_parsed_var_type.type = TY_FLOAT;
        latest_parsed_var_type.bytes = 8; // Temp fix for now
    }
    else if (accept(TK_KW_DOUBLE)) {
        latest_parsed_var_type.type = TY_FLOAT;
        latest_parsed_var_type.bytes = 8;
    }
    else if (accept(TK_KW_VOID)) {
        latest_parsed_var_type.type = TY_VOID;
        latest_parsed_var_type.bytes = 1;
    }
    // Add user defined types here as well
    else {
        return false;
    }
    // We found a type, check if pointer
    while (accept(TK_OP_MULT)) {
        latest_parsed_var_type.ptr_level += 1;
        latest_parsed_var_type.ptr_value_bytes = latest_parsed_var_type.bytes;
        latest_parsed_var_type.bytes = 8;
    }
    // Check for ending [] which implies a pointer level
    if (accept(TK_IDENT)) {
        if(accept(TK_DL_OPENBRACKET)) {
            if (accept(TK_DL_CLOSEBRACKET)) {
                latest_parsed_var_type.ptr_value_bytes = latest_parsed_var_type.bytes;
                latest_parsed_var_type.bytes = 8;
                latest_parsed_var_type.ptr_level++;
                token_go_back(1);
            }
            token_go_back(1);
        }
        token_go_back(1);
    }
    return true;
}

bool accept_literal() {
    return accept(TK_LINT) || accept(TK_LFLOAT) || accept(TK_LCHAR) || accept(TK_LSTRING);
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
    if (accept(TK_IDENT)) { // Global assignment
        token_go_back(1);
        parse_global(node, symbols);
    }
    else {
        Token* cur_parse_token = parse_token;
        expect_type();
        expect(TK_IDENT);
        if (accept(TK_DL_OPENPAREN)) { // Function
            parse_token = cur_parse_token;
            parse_func(node, symbols);
        }
        else { // Global declaration
            parse_token = cur_parse_token;
            parse_global(node, symbols);
        }
    }
    node->next = ast_node_new(AST_END, 1);
    parse_program(node->next, symbols);
}

void parse_func(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_FUNC;
    Function func;
    expect_type();
    expect(TK_IDENT);
    char* ident = prev_token().string_repr;
    func.name = ident;
    func.return_type = latest_parsed_var_type;;
    func.is_defined = false;
    func.is_variadic = false;

    // Create new scope for function
    SymbolTable* func_symbols = symbol_table_create_child(symbols, 0);

    expect(TK_DL_OPENPAREN);
    // Parse function arguments
    while (!accept(TK_DL_CLOSEPAREN)) { // Add argument variables to symbol map
        // Special handling for variadic argument
        if (accept(TK_KW_VARIADIC_DOTS)) {
            func.is_variadic = true;
            continue;
        }
        Variable var; 
        expect_type();
        var.type = latest_parsed_var_type;
        // Check for func(void) arg
        if (latest_parsed_var_type.type == TY_VOID && latest_parsed_var_type.ptr_level == 0) {
            continue;
        }
        // Normal function argument
        expect(TK_IDENT);
        var.name = prev_token().string_repr;
        accept(TK_DL_COMMA);
        symbol_table_insert_var(func_symbols, var);
    }
    // We can directly take the variables pointer, as nothing else will be added
    // in this scope
    func.params = func_symbols->vars;
    func.def_param_count = func_symbols->var_count;

    if (!accept(TK_DL_OPENBRACE)) { // No function body, this is a declaration
        node->type = AST_NULL_STMT; // Definitions are virtual
        node->func = symbol_table_insert_func(symbols, func);
        expect(TK_DL_SEMICOLON);
        return;
    }
    // Function has body, is a definition
    func.is_defined = true;
    latest_func = func;
    node->func = symbol_table_insert_func(symbols, func);
    node->body = ast_node_new(AST_END, 1);
    parse_scope(node->body, func_symbols);
    node->next = ast_node_new(AST_END, 1);
    // Calculate stack space necessary for function
    node->func.stack_space_used = symbol_table_get_max_stack_space(func_symbols);
}

void parse_single_statement(ASTNode* node, SymbolTable* symbols) {
    if (accept(TK_COMMENT)) { // Comment, do nothing, move on to next statement
        parse_single_statement(node, symbols);
        return;
    }
    else if (accept_type()) { // Variable declaration
        // Declaration is just connected to the symbol table,
        // no actual node needed

        // Add to symbol table
        Variable var;
        var.type = latest_parsed_var_type;
        expect(TK_IDENT);
        char* ident = prev_token().string_repr;
        var.name = ident;
        symbol_table_insert_var(symbols, var);

        if (var.type.is_static) { // Static 
            parse_static_declaration(node, symbols);
            return;
        }
        if (accept(TK_DL_OPENBRACKET)) { // Array type
            parse_array_declaration(node, symbols);
            expect(TK_DL_SEMICOLON);
            return;
        }

        if (accept(TK_OP_ASSIGN)) { // Def and assignment
            // Treat this as an expresison
            token_go_back(2); // Go back to ident token
            node->type = AST_EXPR;
            node->top_level_expr = true;
            parse_expression(node, symbols, 1);
            expect(TK_DL_SEMICOLON);
        }
        else {
            expect(TK_DL_SEMICOLON);
            // We can reuse this node, assignment is just virtual
            parse_single_statement(node, symbols);
            return;
        }
    }
    else if (accept(TK_IDENT) || accept_unop()) { // I need to handle literals here too
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
            parse_expression(node, symbols, 1);
            expect(TK_DL_SEMICOLON);
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
        node->cast_type = latest_func.return_type;
        parse_expression(node->ret, symbols, 1);
        expect(TK_DL_SEMICOLON);
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
        node->type = AST_END;
        return;
    }
    parse_single_statement(node, symbols);
    node->next = ast_node_new(AST_STMT, 1);
    parse_statement(node->next, symbols);
}

void parse_scope(ASTNode* node, SymbolTable* symbols) {
    // Make a new child symbol table for this scope
    node->type = AST_SCOPE;
    node->body = ast_node_new(AST_STMT, 1);
    SymbolTable* scope_symbols = symbol_table_create_child(symbols, symbols->cur_stack_offset);
    parse_statement(node->body, scope_symbols);
    node->next = ast_node_new(AST_END, 1);
}

// The end of the expression (, ; is up for the caller to clean up
void parse_expression(ASTNode* node, SymbolTable* symbols, int min_precedence) {
    // Uses precedence climbing
    node->type = AST_EXPR;

    parse_expression_atom(node, symbols);

    while(true) {
        if (accept(TK_DL_OPENBRACKET)) { // Indexing, needs special handling
            parse_binary_op_indexing(node, symbols);
            expect(TK_DL_CLOSEBRACKET);
        }
        else if (accept_binop()) { 
            OpType op_type = token_type_to_bop_type(prev_token().type);
            int op_precedence = get_binary_operator_precedence(op_type);
            if (op_precedence < min_precedence) {
                // We found an end node
                token_go_back(1);
                break;
            }
            // Copy this node to node->lhs
            ASTNode* lhs = ast_node_new(AST_EXPR, 1);
            ast_node_copy(lhs, node);
            node->lhs = lhs;
            node->rhs = ast_node_new(AST_EXPR, 1);
            node->expr_type = EXPR_BINOP;
            node->op_type = op_type;
            int new_min_precedence = op_precedence + !is_binary_operation_assignment(op_type);
            parse_expression(node->rhs, symbols, new_min_precedence);
            // LHS and RHS is now defined. Put the widest variable type in the op
            // for possible implicit casts
            node->cast_type = return_wider_type(node->rhs->cast_type, node->lhs->cast_type);
            if(is_binary_operation_logical(op_type)) {
                // Logical operator, we need to implicitly cast to integer
                // We do this by creating a cast unary op
                ASTNode* rhs = ast_node_new(AST_EXPR, 1);
                ast_node_copy(rhs, node);
                node->expr_type = EXPR_UNOP;
                node->op_type = UOP_CAST;
                node->rhs = rhs;
                node->cast_type.type = TY_INT;
                node->cast_type.ptr_level = 0;
                node->cast_type.is_array = false;
                node->cast_type.bytes = 8;
            };
        }
        else {
            break;
        }
    }
}

void parse_binary_op_indexing(ASTNode* node, SymbolTable* symbols) {
    // Turn current node into deref unop, then rhs into binop of add lhs, rhs
    // a[b] -> *(a+b)
    ASTNode* lhs = ast_node_new(AST_EXPR, 1);
    ast_node_copy(lhs, node);
    ASTNode* rhs = ast_node_new(AST_EXPR, 1);
    parse_expression(rhs, symbols, 1);
    ASTNode* add_binop = ast_node_new(AST_EXPR, 1);
    add_binop->expr_type = EXPR_BINOP;
    add_binop->op_type = BOP_ADD;
    add_binop->rhs = rhs;
    add_binop->lhs = lhs;
    add_binop->cast_type = return_wider_type(rhs->cast_type, lhs->cast_type);
    node->expr_type = EXPR_UNOP;
    node->op_type = UOP_DEREF;
    node->rhs = add_binop;
    node->cast_type = add_binop->cast_type;
    node->cast_type.ptr_level -= 1;
    node->cast_type.is_array = false;
    if (node->cast_type.ptr_level == 0) {
        node->cast_type.bytes = node->cast_type.ptr_value_bytes;
    }
    node->next = ast_node_new(AST_END, 1);
}

void parse_expression_atom(ASTNode* node,  SymbolTable* symbols) {
    // Isolate atom
    if (accept_literal()) { // Literal
        token_go_back(1);
        parse_literal(node, symbols);
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
            node->cast_type = node->var.type;
        }
    }
    else if (accept(TK_DL_OPENPAREN)) {
        if (!accept_type()) { // Just normal parenthesis
            parse_expression(node, symbols, 1);
            expect(TK_DL_CLOSEPAREN);
        }
        else { // This is a type cast unary operator
            node->expr_type = EXPR_UNOP;
            node->op_type = UOP_CAST;
            node->cast_type = latest_parsed_var_type;
            node->rhs = ast_node_new(AST_EXPR, 1);
            expect(TK_DL_CLOSEPAREN);
            parse_expression_atom(node->rhs, symbols);
        }
    }
    // Unary op
    else if (accept_unop()) { // Unary operation
        parse_unary_op(node, symbols);
    }
    else if (accept(TK_DL_CLOSEPAREN) || accept(TK_DL_SEMICOLON)) { 
        // Only scenario this triggers is with a null expression, ex
        // () or ;
        node->type = AST_NULL_STMT;
        node->next = ast_node_new(AST_END, 1);
        token_go_back(1);
    }
    else {
        parse_error("Invalid expression atom");
    }

    while (accept_post_unop()) { // Accept postfix unary operators
        ASTNode* rhs = ast_node_new(AST_EXPR, 1);
        ast_node_copy(rhs, node);
        node->rhs = rhs;
        node->expr_type = EXPR_UNOP;
        node->op_type = token_type_to_post_uop_type(prev_token().type);
    }
}

void parse_unary_op(ASTNode* node, SymbolTable* symbols) {
    node->expr_type = EXPR_UNOP;
    node->op_type = token_type_to_pre_uop_type(prev_token().type);
    node->rhs = ast_node_new(AST_EXPR, 1);
    node->next = ast_node_new(AST_END, 1);

    if (node->op_type == UOP_SIZEOF) {
        // This is either a type or an expression
        // Expressions need to evaluated normally, then the topmost node
        // will provide the most memory used. This needs to be passed up,
        // will help with implicit conversions later as well. Should be in the op node
        expect(TK_DL_OPENPAREN);
        if (accept_type()) {
            node->rhs->cast_type = latest_parsed_var_type;
            expect(TK_DL_CLOSEPAREN);
        }
        else {
            token_go_back(1);
            parse_expression_atom(node->rhs, symbols);
        }
        node->rhs->type = AST_END;
        node->cast_type.type = TY_INT;
        node->cast_type.bytes = 8;
    }
    else {
        parse_expression_atom(node->rhs, symbols);
        node->cast_type = node->rhs->cast_type;
        if (node->op_type == UOP_ADDR) { // This changes cast_type
            if (node->cast_type.ptr_level == 1) {
                node->cast_type.ptr_value_bytes = node->cast_type.bytes;
            } 
            node->cast_type.ptr_level += 1;
        }
        else if(node->op_type == UOP_DEREF) {
            if (node->cast_type.ptr_level == 0) {
                parse_error("Attempting to dereference non-pointer type!");
            } 
            else if(node->cast_type.ptr_level == 1) {
                node->cast_type.bytes = node->cast_type.ptr_value_bytes;
                node->var = node->rhs->var;
                node->var.is_dereferenced_ptr = true;
            }
            node->cast_type.ptr_level -= 1;
            node->cast_type.is_array = false;
        }
    }
}

void parse_literal(ASTNode* node,  SymbolTable* symbols) {
    node->expr_type = EXPR_LITERAL;
    node->literal = parse_token->string_repr;
    node->cast_type.bytes = 0;
    if (accept(TK_LINT)) {
        node->cast_type.type = TY_INT;
        node->literal_type = LT_INT;
    }
    else if (accept(TK_LFLOAT)) {
        node->cast_type.type = TY_FLOAT;
        node->literal_type = LT_FLOAT;
    }
    else if (accept(TK_LCHAR)) {
        node->cast_type.type = TY_INT;
        node->cast_type.ptr_level = 1;
        node->literal_type = LT_CHAR;
    }
    else if (accept(TK_LSTRING)) {
        node->cast_type.type = TY_INT;
        node->literal_type = LT_STRING;
    }
}

void parse_global(ASTNode* node, SymbolTable* symbols) {
    if (accept(TK_IDENT)) { // Definition of already declared global variable
        char* ident = prev_token().string_repr;
        token_go_back(1);
        parse_expression(node, symbols, 1);
        expect(TK_DL_SEMICOLON);
        if (!is_valid_const_assignment(node, symbols)) {
                parse_error("Non-constant global expression found");
        }
        Variable* inserted_var = symbol_table_lookup_var_ptr(symbols, ident);
        inserted_var->const_expr = evaluate_const_assignment(node, symbols);
        inserted_var->const_expr_type = node->rhs->literal_type;
        inserted_var->is_undefined = false;
        node->type = AST_NULL_STMT; // Declaration is virtual
    }
    else {
        accept_type();
        Variable var;
        var.type = latest_parsed_var_type;
        expect(TK_IDENT);
        char* ident = prev_token().string_repr;
        var.name = ident;
        var.is_undefined = true;
        var.is_global = true;
        symbol_table_insert_var(symbols, var);
        if (accept(TK_DL_OPENBRACKET)) { // Array type
            parse_array_declaration(node, symbols);
            expect(TK_DL_SEMICOLON);
            return;
        }
        if (accept(TK_OP_ASSIGN)) {
            token_go_back(2);
            parse_expression(node, symbols, 1);
            if (!is_valid_const_assignment(node, symbols)) {
                parse_error("Non-constant global expression found");
            }
            Variable* inserted_var = symbol_table_lookup_var_ptr(symbols, ident);
            inserted_var->const_expr = evaluate_const_assignment(node, symbols);
            inserted_var->const_expr_type = node->rhs->literal_type;
            inserted_var->is_undefined = false;
        }
        expect(TK_DL_SEMICOLON);
        node->type = AST_NULL_STMT; // This is just a virtual node
    }
}

void parse_static_declaration(ASTNode* node, SymbolTable* symbols) {
    char* ident = prev_token().string_repr;
    Variable* var = symbol_table_lookup_var_ptr(symbols, ident);

    if (accept(TK_OP_ASSIGN)) { // Def and assignment
        // Treat this as an expresison
        token_go_back(2); // Go back to ident token
        node->type = AST_EXPR;
        node->top_level_expr = true;
        parse_expression(node, symbols, 1);
        if (!is_valid_const_assignment(node, symbols)) {
            parse_error("Attempted to initialize static variable with non-const value!");
        }
        var->const_expr = evaluate_const_assignment(node, symbols);
        var->const_expr_type = node->rhs->literal_type;
        var->is_undefined = false;
        expect(TK_DL_SEMICOLON);
        node->type = AST_NULL_STMT;
    }
    else {
        var->is_undefined = true;
        var->type.is_static = true;
        if (accept(TK_DL_OPENBRACKET)) { // Array type
            parse_array_declaration(node, symbols);
            expect(TK_DL_SEMICOLON);
            return;
        }
        expect(TK_DL_SEMICOLON);
        // We can reuse this node, assignment is just virtual
        parse_single_statement(node, symbols);
        node->type = AST_NULL_STMT;
    }
}

void parse_array_declaration(ASTNode* node, SymbolTable* symbols) {
    token_go_back(1);
    node->type = AST_NULL_STMT; // Declarations are virtual
    char* ident = prev_token().string_repr;
    Variable* var = symbol_table_lookup_var_ptr(symbols, ident);
    var->type.is_array = true;
    var->type.ptr_level++;
    if (var->type.ptr_value_bytes == 0) {
        var->type.ptr_value_bytes = var->type.bytes;
    }
    var->type.bytes = 8;
    var->type.array_has_initializer = false;
    // Allocate stackspace for array
    ASTNode* temp_node = ast_node_new(AST_EXPR, 1);
    expect(TK_DL_OPENBRACKET);
    parse_expression(temp_node, symbols, 1);
    if (!is_const_expression(temp_node, symbols)) {
        parse_error("Attempted to declare array with non-const size!");
    }
    char* const_expr = evaluate_const_expression(temp_node, symbols);
    var->type.array_size = atoi(const_expr);
    symbols->cur_stack_offset += var->type.bytes * var->type.array_size;
    var->stack_offset = symbols->cur_stack_offset;
    if (symbols->is_global) {
        var->is_global = true;
    }
    expect(TK_DL_CLOSEBRACKET);

    if (accept(TK_OP_ASSIGN)) { // Initializer
        var->type.array_has_initializer = true;
        node->var = *var;
        parse_array_initializer(node, symbols);
    }
}

void parse_array_initializer(ASTNode* node, SymbolTable* symbols) {
    expect(TK_DL_OPENBRACE);
    node->type = AST_INIT;
    node->args = ast_node_new(AST_EXPR, 1);
    ASTNode* arg_node = node->args;
    while (!(accept(TK_DL_CLOSEBRACE)) || prev_token().type == TK_DL_OPENBRACE) { // Go through initializer args
        parse_expression(arg_node, symbols, 1);
        if (!is_const_expression(arg_node, symbols)) {
            parse_error("Non-constant element found in array initializer!");
        }
        arg_node->next = ast_node_new(AST_END, 1);
        arg_node = arg_node->next;
        accept(TK_DL_COMMA);
    }
}

void parse_func_call(ASTNode* node, SymbolTable* symbols) {
    node->expr_type = EXPR_FUNC_CALL;
    char* ident = prev_token().string_repr;
    expect(TK_DL_OPENPAREN);
    node->func = symbol_table_lookup_func(symbols, ident);
    node->args = ast_node_new(AST_EXPR, 1);
    node->cast_type = node->func.return_type; // Return type
    ASTNode* arg_node = node->args;
    int arg_count = 0;
    while (!(accept(TK_DL_CLOSEPAREN) || prev_token().type == TK_DL_CLOSEPAREN)) { // Go through argument expressions
        parse_expression(arg_node, symbols, 1);
        arg_node->next = ast_node_new(AST_END, 1);
        arg_node->next->prev = arg_node;
        arg_node = arg_node->next;
        accept(TK_DL_COMMA);
        arg_count++;
        if (arg_count > node->func.def_param_count && !node->func.is_variadic) {
            parse_error("Function call error, too many parameters!");
        }
    }
    node->func.call_param_count = arg_count;
    node->args_end = arg_node;
}

void parse_if(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_IF;
    node->cond = ast_node_new(AST_EXPR, 1);
    expect(TK_DL_OPENPAREN);
    parse_expression(node->cond, symbols, 1);
    expect(TK_DL_CLOSEPAREN);
    symbols->cur_stack_offset += 8;
    node->body = ast_node_new(AST_SCOPE, 1);
    parse_single_statement(node->body, symbols);
    node->body->next = ast_node_new(AST_END, 1);
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
    parse_expression(node->cond, symbols, 1);
    expect(TK_DL_CLOSEPAREN);
    symbols->cur_stack_offset += 8;
    node->body = ast_node_new(AST_SCOPE, 1);
    parse_single_statement(node->body, symbols);
    node->body->next = ast_node_new(AST_END, 1);
}

// Parse a do while loop
void parse_do_while_loop(ASTNode* node, SymbolTable* symbols) {
    node->type = AST_DO_LOOP;
    // Parse do while body
    node->body = ast_node_new(AST_SCOPE, 1);
    parse_single_statement(node->body, symbols);
    node->body->next = ast_node_new(AST_END, 1);
    // Parse while condition at end
    expect(TK_KW_WHILE);
    expect(TK_DL_OPENPAREN);
    symbols->cur_stack_offset += 8;
    node->cond = ast_node_new(AST_EXPR, 1);
    parse_expression(node->cond, symbols, 1);
    expect(TK_DL_CLOSEPAREN);
    expect(TK_DL_SEMICOLON);
}

// Parse a for loop
void parse_for_loop(ASTNode* node, SymbolTable* symbols) {
    // Same construction as while loop, but we need to insert a few additional statements
    // for(int i = 0; i < 10, i++) becomes
    // int i = 0; while(i < 10) { i++; ... }

    // We need a new scope for the for variable declaration
    node->type = AST_SCOPE;
    node->body = ast_node_new(AST_STMT, 1);
    SymbolTable* scope_symbols = symbol_table_create_child(symbols, symbols->cur_stack_offset);
    node->next = ast_node_new(AST_STMT, 1);

    // Set the first statement to the for init statement
    expect(TK_DL_OPENPAREN);
    parse_single_statement(node->body, scope_symbols);

    // Now we can treat this like a while loop almost
    node->body->next = ast_node_new(AST_LOOP, 1);
    ASTNode* loop_node = node->body->next;
    loop_node->next = ast_node_new(AST_END, 1);

    // Getting the condition
    loop_node->cond = ast_node_new(AST_EXPR, 1);
    scope_symbols->cur_stack_offset += 8;
    parse_expression(loop_node->cond, scope_symbols, 1);
    accept(TK_DL_SEMICOLON);

    // Getting the increment expression
    loop_node->incr = ast_node_new(AST_EXPR, 1);
    scope_symbols->cur_stack_offset += 8;
    parse_expression(loop_node->incr, scope_symbols, 1);
    loop_node->incr->next = ast_node_new(AST_END, 1);
    expect(TK_DL_CLOSEPAREN);

    // Parsing the for-loop body
    loop_node->body = ast_node_new(AST_STMT, 1);
    parse_single_statement(loop_node->body, scope_symbols);

    // We need to insert the increment operation last
    loop_node->body->next = ast_node_new(AST_END, 1);
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
    parse_expression(node->cond, switch_symbols,1 );
    expect(TK_DL_CLOSEPAREN);
    // Now we can parse the contents
    node->body = ast_node_new(AST_STMT, 1);
    node->body->next = ast_node_new(AST_END, 1);
    parse_single_statement(node->body, switch_symbols);
    // We now need to grab the case labels and store them in the AST Node
    node->switch_cases = symbol_table_lookup_switch_case_labels(switch_symbols);
    node->next = ast_node_new(AST_END, 1);
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
    static char* RED_COLOR_STR = "\033[31;1m";
    static char* RESET_COLOR_STR = "\033[0m";
    fprintf(stderr, "%sParse error:%s %s\n", RED_COLOR_STR, RESET_COLOR_STR, error_message);
    fprintf(stderr, "At line %d |  %s %s%s%s %s    (at token: \"%s\")\n", 
            parse_token->src_line,
            (parse_token - 1)->string_repr,
            RED_COLOR_STR,
            parse_token->string_repr,
            RESET_COLOR_STR,
            (parse_token + 1)->string_repr,
            token_type_to_string(parse_token->type)
    );
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


bool is_const_expression(ASTNode* node, SymbolTable* symbols) {
    return (node->type == AST_EXPR && 
            node->expr_type == EXPR_LITERAL);
}

bool is_valid_const_assignment(ASTNode* node, SymbolTable* symbols) {
    return (node->type == AST_EXPR && 
            node->expr_type == EXPR_BINOP && 
            node->op_type == BOP_ASSIGN &&
            is_const_expression(node->rhs, symbols));
}

char* evaluate_const_expression(ASTNode* node, SymbolTable* symbols) {
    return node->literal;
}

// Evaluate a constant assignment expression
char* evaluate_const_assignment(ASTNode* node, SymbolTable* symbols) {
    return node->rhs->literal;
}

// Issue: Currently I just set the operation to be in int, which I don't want
// But I want the result to be casted to this. 3.0 < 1 needs the cast type to be correct, right?

// Get the precedence level of a binary operator
// The higher the number, the higher the precedence
int get_binary_operator_precedence(OpType type) {
    switch (type) {
            case BOP_INDEX:
                return 14;
            case BOP_DIV:         // /
            case BOP_MUL:         // *
            case BOP_MOD:         // %
                return 13;
            case BOP_ADD:         // +
            case BOP_SUB:         // -
                return 12;
            case BOP_LEFTSHIFT:   // <<
            case BOP_RIGHTSHIFT:  // >>
                return 11;
            case BOP_LT:          // <
            case BOP_LTE:         // <=
            case BOP_GT:          // <
            case BOP_GTE:         // <=
                return 10;
            case BOP_EQ:          // ==
            case BOP_NEQ:         // !=
                return 9;
            case BOP_BITAND:      // &
                return 8;
            case BOP_BITXOR:      // ^
                return 7;
            case BOP_BITOR:       // |
                return 6;
            case BOP_AND:         // &&
                return 5;
            case BOP_OR:          // ||
                return 4;
            // 3 = ternary
            case BOP_ASSIGN:      // =
            case BOP_ASSIGN_ADD:
            case BOP_ASSIGN_SUB:
            case BOP_ASSIGN_MULT:
            case BOP_ASSIGN_DIV:
            case BOP_ASSIGN_MOD:
            case BOP_ASSIGN_LEFTSHIFT:
            case BOP_ASSIGN_RIGHTSHIFT:
            case BOP_ASSIGN_BITAND:
            case BOP_ASSIGN_BITOR:
            case BOP_ASSIGN_BITXOR:
                return 2;
            // 1 = comma
            default:
                return 0;
    }
}

// Assignment is right associative, needed for the precedence parsing
bool is_binary_operation_assignment(OpType type) {
    switch (type) {
            case BOP_ASSIGN:
            case BOP_ASSIGN_ADD:
            case BOP_ASSIGN_SUB:
            case BOP_ASSIGN_MULT:
            case BOP_ASSIGN_DIV:
            case BOP_ASSIGN_MOD:
            case BOP_ASSIGN_LEFTSHIFT:
            case BOP_ASSIGN_RIGHTSHIFT:
            case BOP_ASSIGN_BITAND:
            case BOP_ASSIGN_BITOR:
            case BOP_ASSIGN_BITXOR:
                return true;
            default:
                return false;
    }
}

// Return whether the binary operation is of a logical type (&&, ||, > etc)
bool is_binary_operation_logical(OpType type) {
    switch (type) {
        case BOP_EQ: // Equals
        case BOP_NEQ: // Not equals
        case BOP_LT: // Less than
        case BOP_LTE: // Less than equals
        case BOP_GT: // Greater than
        case BOP_GTE: // Greater than equals
            return true;
        default:
            return false;
    }
}

VarType return_wider_type(VarType type1, VarType type2) {
    int type1_width = type1.bytes;
    int type2_width = type2.bytes;
    if (type1.type == TY_FLOAT) {
        type1_width += 8;
    }
    if (type2.type == TY_FLOAT) {
        type2_width += 8;
    }
    if (type1_width > type2_width) {
        return type1;
    }
    else {
        return type2;
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
        case TK_OP_SIZEOF:
            return UOP_SIZEOF;
        case TK_OP_BITAND:
            return UOP_ADDR;
        case TK_OP_MULT:
            return UOP_DEREF;
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
        case TK_OP_BITOR:
            return BOP_BITOR;
        case TK_OP_BITAND:
            return BOP_BITAND;
        case TK_OP_BITXOR:
            return BOP_BITXOR;
        case TK_OP_LEFTSHIFT:
            return BOP_LEFTSHIFT;
        case TK_OP_RIGHTSHIFT:
            return BOP_RIGHTSHIFT;
        case TK_OP_ASSIGN:
            return BOP_ASSIGN;
        case TK_OP_ASSIGN_ADD:
            return BOP_ASSIGN_ADD;
        case TK_OP_ASSIGN_SUB:
            return BOP_ASSIGN_SUB;
        case TK_OP_ASSIGN_MULT:
            return BOP_ASSIGN_MULT;
        case TK_OP_ASSIGN_DIV:
            return BOP_ASSIGN_DIV;
        case TK_OP_ASSIGN_MOD:
            return BOP_ASSIGN_MOD;
        case TK_OP_ASSIGN_LEFTSHIFT:
            return BOP_ASSIGN_LEFTSHIFT;
        case TK_OP_ASSIGN_RIGHTSHIFT:
            return BOP_ASSIGN_RIGHTSHIFT;
        case TK_OP_ASSIGN_BITAND:
            return BOP_ASSIGN_BITAND;
        case TK_OP_ASSIGN_BITOR:
            return BOP_ASSIGN_BITOR;
        case TK_OP_ASSIGN_BITXOR:
            return BOP_ASSIGN_BITXOR;
        case TK_DL_OPENBRACKET:
            return BOP_INDEX;
        default:
            parse_error("Unsupported binary operation encountered while parsing");
            return 0;
    }
}