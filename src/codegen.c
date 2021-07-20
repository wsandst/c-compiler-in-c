#include "codegen.h"

StrVector asm_src;
char* asm_indent_str;

void asm_add(char* str) {
    char* new_str1 = str_add(str, "\n");
    char* new_str2 = str_add(asm_indent_str, new_str1);
    free(new_str1);
    str_vec_push_no_copy(&asm_src, new_str2);
}

void asm_add_two(char* str1, char* str2) {
    char* combined = str_add(str1, str2);
    asm_add(combined);
    free(combined);
}

void asm_add_return(char* return_val) {
    asm_add_two("mov rax, ", return_val);
}

void asm_set_indent(int indent) {
    free(asm_indent_str);
    asm_indent_str = str_multiply("\t", indent);
}

char* generate_assembly(AST* ast) {
    asm_src = str_vec_new(16);
    asm_indent_str = calloc(1, sizeof(char));

    // Setup globals/functions
    asm_add("global main");
    asm_add("section .text");

    gen_asm(ast->program);

    char* asm_src_str = str_vec_join(&asm_src);
    str_vec_free(&asm_src);
    return asm_src_str;
}

void gen_asm(ASTNode* node) {
    switch (node->type) {
        case AST_FUNC:
            asm_add_two(node->func->name, ":");
            asm_set_indent(1);
            gen_asm(node->body);
            break;
        case AST_RETURN:
            asm_add_return(node->ret->literal);
            return;
        default:
            return;
    }
}