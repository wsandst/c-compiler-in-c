#include "codegen.h"

StrVector asm_src;
char* asm_indent_str;

void asm_add(char* str) {
    char* new_str1 = str_add(str, "\n");
    char* new_str2 = str_add(asm_indent_str, new_str1);
    free(new_str1);
    str_vec_push_no_copy(&asm_src, new_str2);
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

    // Code
    asm_add("section .text");
    asm_add("main:");
    asm_set_indent(1);
    asm_add("mov rax, 1");
    asm_add("ret"); 

    char* asm_src_str = str_vec_join(&asm_src);
    str_vec_free(&asm_src);
    return asm_src_str;
}