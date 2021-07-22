#include "codegen.h"

StrVector asm_src;
char* asm_indent_str;

void asm_add_single(char* str) {
    str_vec_push(&asm_src, str);
}

void asm_add(int n, ...) {
    char* str;
    va_list vl;
    va_start(vl, n);
    for (int i = 0; i < n; i++)
    {
        str = va_arg(vl, char*);
        asm_add_single(str);
    }
    va_end(vl);
}

void asm_add_nl() {
    char buf[64];
    snprintf(buf, 63, "\n%s", asm_indent_str);
    asm_add_single(buf);
}

void asm_add_return(char* return_val) {
    asm_add(2, "mov rax, ", return_val);
    asm_add_nl();
    asm_add(1, "pop rbp");
    asm_add_nl();
    asm_add(1, "ret");
    asm_add_nl();
}

void asm_set_indent(int indent) {
    free(asm_indent_str);
    asm_indent_str = str_multiply("\t", indent);
}

char* var_to_stack_ptr(Variable* var) {
    char buf[64];
    snprintf(buf, 63, "qword[rbp-%i]", var->stack_offset);
    return str_copy(buf);
}

char* generate_assembly(AST* ast) {
    asm_src = str_vec_new(16);
    asm_indent_str = calloc(1, sizeof(char));

    // Setup globals/functions
    asm_add(1, "global main\n");
    asm_add(1, "section .text\n");

    gen_asm(ast->program);

    char* asm_src_str = str_vec_join(&asm_src);
    str_vec_free(&asm_src);
    free(asm_indent_str);
    return asm_src_str;
}

void gen_asm(ASTNode* node) {
    switch (node->type) {
        case AST_PROGRAM:
            gen_asm(node->body);
            return;
        case AST_FUNC:
            asm_set_indent(0);
            asm_add_nl();
            asm_add(2, node->func.name, ":");
            asm_set_indent(1);
            asm_add_nl();
            asm_add(1, "push    rbp");
            asm_add_nl();
            asm_add(1, "mov rbp, rsp");
            asm_add_nl();
            gen_asm(node->body);
            gen_asm(node->next);
            break;
        case AST_BLOCK:
            gen_asm(node->body);
            gen_asm(node->next);
            break;
        case AST_NONE:
        case AST_END:
            return;
        case AST_RETURN:
            if (node->ret->type == AST_NUM) {
                asm_add_return(node->ret->literal);
            }
            else if (node->ret->type == AST_VAR) {
                char* sp = var_to_stack_ptr(&node->ret->var);
                asm_add_return(sp);
                free(sp);
            }
            return;
        case AST_ASSIGN: 
            {
            char* sp = var_to_stack_ptr(&node->var);
            if (node->assign->type == AST_NUM) {
                asm_add(4, "mov ", sp, ", ", node->assign->literal);
                asm_add_nl();
            }
            else if (node->assign->type == AST_VAR) {
                char* sp2 = var_to_stack_ptr(&node->assign->var);
                asm_add(2, "mov rax, ", sp2);
                asm_add_nl();
                asm_add(3, "mov ", sp, ", rax");
                asm_add_nl();
                free(sp2);
            }
            free(sp);
            }
            gen_asm(node->next);
            break;
        case AST_VAR_DEC:
            // Do nothing
            gen_asm(node->next);
            break;
        default:
            return;
    }
}