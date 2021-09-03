#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../src/tokens.h"
#include "../../src/preprocess.h"
#include "../../src/util/file_helpers.h"
#include "../../src/symbol_table.h"
#include "../../src/parser.h"

void test_parser();
void test_parser_helpers();
void test_parser_on_file();

void test_parser() {
    printf("[CTEST] Running parser tests...\n");
    test_parser_helpers();
    test_parser_on_file();
    printf("[CTEST] Passed parser tests!\n");
}

void test_parser_helpers() {
    // Test parser helper functions
    Token* tokens = calloc(3, sizeof(Token));
    tokens[0].type = TK_KW_WHILE;
    tokens[1].type = TK_KW_FOR;
    set_parse_token(tokens);
    assert(!accept(TK_KW_FOR));
    assert(accept(TK_KW_WHILE));
    assert(accept(TK_KW_FOR));
    token_go_back(2);
    expect(TK_KW_WHILE);
    free(tokens);

    ASTNode* node1 = ast_node_new(AST_LOOP, 1);
    assert(node1->type == AST_LOOP);
    ASTNode* node2 = ast_node_new(AST_EXPR, 1);
    node1->literal = "test";
    ast_node_copy(node2, node1);
    assert(node2->type == AST_LOOP);
    assert(strcmp(node2->literal, "test") == 0);
    node2->next_mem = NULL;
    ast_node_free(node1);

    assert(token_type_to_bop_type(TK_OP_AND) == BOP_AND);
    assert(token_type_to_bop_type(TK_OP_MINUS) == BOP_SUB);
    assert(token_type_to_pre_uop_type(TK_OP_MINUS) == UOP_NEG);
}

void test_parser_on_file() {
    // Test parser on example file
    PreprocessorTable table = preprocessor_table_new();
    Tokens tokens = preprocess_first("test/unit/examples/example2.c", &table);

    SymbolTable* symbols = symbol_table_new();
    AST ast = parse(&tokens, symbols);

    // Validate the AST manually
    ASTNode* node = ast.program;
    assert(node->type == AST_PROGRAM);
    node = node->body;
    assert(node->type == AST_FUNC);
    assert(node->next->type == AST_END);
    node = node->body;
    assert(node->type == AST_SCOPE);
    node = node->body;
    assert(node->type == AST_RETURN);
    node = node->ret;
    assert(node->type == AST_EXPR);
    assert(node->expr_type == EXPR_BINOP);
    assert(node->op_type == BOP_ADD);
    ASTNode* lhs = node->lhs;
    ASTNode* rhs = node->rhs;
    assert(lhs->expr_type == EXPR_LITERAL);
    assert(strcmp(lhs->literal, "0") == 0);
    assert(rhs->expr_type == EXPR_BINOP);
    assert(rhs->op_type == BOP_MUL);
    lhs = rhs->lhs;
    rhs = rhs->rhs;
    assert(lhs->expr_type == EXPR_LITERAL);
    assert(strcmp(lhs->literal, "1") == 0);
    assert(rhs->expr_type == EXPR_LITERAL);
    assert(strcmp(rhs->literal, "2") == 0);

    symbol_table_free(symbols);
    preprocessor_table_free(&table);
    tokens_free(&tokens);
    tokens_free_line_strings(&tokens);
    ast_free(&ast);
}