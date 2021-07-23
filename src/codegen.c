#include "codegen.h"

// Toggle for including comments in the asm output code
const bool INCLUDE_COMMENTS = true;

StrVector asm_src;
char* asm_indent_str;

void asm_add_single(char* str) {
    str_vec_push(&asm_src, str);
}

void asm_add(int n, ...) {
    asm_add_newline();
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

void asm_add_com(char* comment) {
    if (INCLUDE_COMMENTS) {
        asm_add_newline();
        asm_add_single(comment);
    }
}

void asm_add_newline() {
    char buf[64];
    snprintf(buf, 63, "\n%s", asm_indent_str);
    asm_add_single(buf);
}

void asm_set_indent(int indent) {
    free(asm_indent_str);
    asm_indent_str = str_multiply("    ", indent);
}

void codegen_error(char* error_message) {
    fprintf(stderr, "Codegen error: %s\n", error_message);
    // We are not manually freeing the memory here, 
    // but as the program is exiting it is fine
    exit(1); 
}

char* offset_to_stack_ptr(int offset) {
    char buf[64];
    snprintf(buf, 63, "qword[rbp-%i]", offset);
    return str_copy(buf);
}


char* var_to_stack_ptr(Variable* var) {
    return offset_to_stack_ptr(var->stack_offset);
}

char* generate_assembly(AST* ast) {
    asm_src = str_vec_new(16);
    asm_indent_str = calloc(1, sizeof(char));

    // Setup globals/functions
    asm_set_indent(0);
    asm_add(1, "global main");
    asm_add(1, "section .text");

    gen_asm(ast->program);
    asm_add_newline();

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
            asm_add_newline();
            asm_add(2, node->func.name, ":");
            asm_set_indent(1);
            asm_add_com("; Setting up function stack pointer");
            asm_add(1, "push rbp");
            asm_add(1, "mov rbp, rsp");
            asm_add_com("; Allocate a hardcoded 128 byte stack allocation per function");
            asm_add(1, "sub rsp, 128"); // Hardcoded 128 bytes on the stack for the function 
            // I need to start allocating stack space here,
            // currently only using the red zone
            //asm_add(1, "sub rsp, 16"); // Allocate space for variables on stack
            asm_add_com("; Function code start");
            // Do I need to allocate more stack space here?
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
            // Manual handling of expressions, this should be done another way
            if (node->ret->type == AST_EXPR) {
                asm_add_com("; Evaluating return expr");
                gen_asm(node->ret); // Expr is now in RAX
                asm_add_com("; Function return");
                asm_add_com("; Restore hardcoded 128 byte stack allocation per function");
                asm_add(1, "add rsp, 128");
                asm_add(1, "pop rbp");
                asm_add(1, "ret");
            }
            return;
        case AST_ASSIGN: 
            // Manual handling of expressions, this should be done another way
            // This should be a separate expr gen_asm()
            if (node->assign->type == AST_EXPR) {
                char* sp = var_to_stack_ptr(&node->var);
                gen_asm(node->assign);
                asm_add(3, "mov ", sp, ", rax");
                free(sp);
            }
            gen_asm(node->next);
            break;
        case AST_VAR_DEC:
            // Do nothing
            gen_asm(node->next);
            break;
        case AST_EXPR:
            if (node->expr_type == EXPR_LITERAL) {
                asm_add(2, "mov rax, ", node->literal);
            }
            else if (node->expr_type == EXPR_VAR) {
                char* sp2 = var_to_stack_ptr(&node->var);
                asm_add(2, "mov rax, ", sp2);
                free(sp2);
            }
            else if (node->expr_type == EXPR_FUNC_CALL) { // Function call
                asm_add_com("; Expression function call");
                //asm_add(1, "sub rsp, 16"); // Allocate space for parameters on stack
                asm_add(2, "call ", node->func.name);
            }
            else if (node->expr_type == EXPR_UNOP) {
                gen_asm_unary_op(node);
            }
            else if (node->expr_type == EXPR_BINOP) {
                gen_asm_binary_op(node);
            }
            else {
                codegen_error("Non-supported expression type encountered!");
            }
            break;
        default:
            codegen_error("Encountered AST Node which has no codegen capability yet!");
            return;
    }
}

void gen_asm_unary_op(ASTNode* node) {
    gen_asm(node->rhs); // The value we are acting on is now in RAX
    switch (node->op_type) {
        case UOP_NEG: // Negation
            asm_add(1, "neg rax");
            break;
        case UOP_COMPL: // Complement
            asm_add(1, "not rax");
            break;
        case UOP_NOT: // Logical not
            asm_add(1, "cmp rax, 0");
            asm_add(1, "mov rax, 0");
            asm_add(1, "sete al");
            break;
        default:
            codegen_error("Unsupported unary operation found!");
            break;
    }
}

void gen_asm_binary_op(ASTNode* node) {
    // The op needs to know which register/address to save the results in so we don't conflict
    gen_asm(node->lhs); // LHS now in RAX
    char* sp = offset_to_stack_ptr(node->scratch_stack_offset);
    asm_add(3, "mov ", sp, ", rax");
    gen_asm(node->rhs); // RHS now in RAX
    asm_add(1, "mov rbx, rax"); // Move RHS to RBX
    asm_add(2, "mov rax, ", sp); // Move saved LHS to RAX
    // We are now ready for the binary operation
    switch (node->op_type) {
        case BOP_ADD: // Addition
            asm_add(1, "add rax, rbx");
            break;
        case BOP_SUB: // Subtraction
            asm_add(1, "sub rax, rbx");
            break;
        case BOP_MUL: // Multiplication
            asm_add(1, "imul rax, rbx");
            break;
        default:
            codegen_error("Unsupported binary operation found!");
            break;
    }
}
