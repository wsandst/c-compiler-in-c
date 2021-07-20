#include "codegen.h"

// Todo: Symbol map. This can be an array. Every time we encounter a new variable,
// we create an entry here. We need some way of mapping to whatever variable is here,
// when we encounter a usage of that variable. How though?
/*
// This is valid code
{
    int y = 5;
}
{
    int y = 5;
}
*/
// These objects 

StrVector asm_src;
char* asm_indent_str;

void asm_add(char* str) {
    char* new_str1 = str_add(str, "\n");
    char* new_str2 = str_add(asm_indent_str, new_str1);
    free(new_str1);
    str_vec_push_no_copy(&asm_src, new_str2);
    {
        int y = 5;
    }
    {
        int y = 5;
    }
}

void asm_add_return(char* return_val) {
    asm_add(str_add("mov rax, ", return_val));
    asm_add("ret"); 
}

void asm_set_indent(int indent) {
    free(asm_indent_str);
    asm_indent_str = str_multiply("\t", indent);
}

char* generate_assembly(AST *ast) {
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

void gen_asm(ASTNode *node) {
    if (node->type == AST_FUNC && strcmp(node->func->name, "main") == 0) {
        asm_add("main:");
        asm_set_indent(1);
        gen_asm(node->body);
    }
    else if (node->type == AST_RETURN) {
        asm_add_return(node->ret->literal);
        return;
    }
}