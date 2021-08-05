#include "codegen.h"

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node, AsmContext ctx) {
    if (node->expr_type == EXPR_LITERAL) {
        if (node->literal_type == LT_INT) {
            asm_add(2, "mov rax, ", node->literal);
        }
        else if (node->literal_type == LT_FLOAT) {
            asm_add(3, "mov rax, __float64__(", node->literal, ")");
            asm_add(1, "movq xmm0, rax");
        }
        else {
            codegen_error("Unsupported literal encountered");
        }
    }
    else if (node->expr_type == EXPR_VAR) {
        char* sp2 = var_to_stack_ptr(&node->var);
        // Handle various variable type sizes
        if (node->var.type.type == TY_INT) {
            if (node->var.type.bytes == 1 || node->var.type.bytes == 2) {
                asm_add(2, "movzx rax, ", sp2); // Zeroes the upper unused bits
            }
            else if (node->var.type.bytes == 4) {
                asm_add(2, "mov eax, ", sp2); // Moving into eax automatically zeroes upper bits
            }
            else if (node->var.type.bytes == 8) {
                asm_add(2, "mov rax, ", sp2);
            }
        }
        else if (node->var.type.type == TY_FLOAT) {
            asm_add(2, "movq xmm0, ", sp2);
        }
        else {
            codegen_error("Unsupported variable type encountered");
        }
        free(sp2);
    }
    else if (node->expr_type == EXPR_FUNC_CALL) { // Function call
        gen_asm_func_call(node, ctx);
        if (node->top_level_expr) {
            gen_asm(node->next, ctx);
        }
    }
    else if (node->expr_type == EXPR_UNOP) {
        gen_asm_unary_op(node, ctx);
        if (node->top_level_expr) {
            gen_asm(node->next, ctx);
        }
    }
    else if (node->expr_type == EXPR_BINOP) {
        gen_asm_binary_op(node, ctx);
        if (node->top_level_expr) {
            gen_asm(node->next, ctx);
        }
    }
    else {
        codegen_error("Non-supported expression type encountered!");
    }
}

void gen_asm_unary_op(ASTNode* node, AsmContext ctx) {
    if (node->cast_type.type == TY_INT) {
        gen_asm_unary_op_int(node, ctx);
    }
    else if (node->cast_type.type == TY_FLOAT) {
        gen_asm_unary_op_float(node, ctx);
    }
    else {
        codegen_error("Invalid cast-type encountered");
    }
}
void gen_asm_binary_op(ASTNode* node, AsmContext ctx) {
    if (node->cast_type.type == TY_INT) {
        // We need to perform implicit casting here
        gen_asm_binary_op_int(node, ctx);
    }
    else if (node->cast_type.type == TY_FLOAT) {
        gen_asm_binary_op_float(node, ctx);
    }
    else {
        codegen_error("Invalid cast-type encountered");
    }
}

// =============== Integer operations ===============

void gen_asm_unary_op_int(ASTNode* node, AsmContext ctx) {
    gen_asm(node->rhs, ctx); // The value we are acting on is now in RAX
    char* var_sp = var_to_stack_ptr(&node->rhs->var);
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
        // Increment, decrement
        // This is kind of a form of assignment
        case UOP_PRE_INCR: // ++x
            // Increment and return incremented value
            if (node->rhs->expr_type != EXPR_VAR) {
                codegen_error("Only variables can be incremented to");
            }
            asm_add_com("; Op: ++ (pre)");
            asm_add(1, "inc rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            break;
        case UOP_PRE_DECR: // --x
            // Decrement and return incremented value
            if (node->rhs->expr_type != EXPR_VAR) {
                codegen_error("Only variables can be decremented to");
            }
            asm_add_com("; Op: -- (pre)");
            asm_add(1, "dec rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            break;
        case UOP_POST_INCR: // x++
            // Increment and return previous value
            if (node->rhs->expr_type != EXPR_VAR) {
                codegen_error("Only variables can be incremented to");
            }
            asm_add_com("; Op: ++ (post)");
            asm_add(1, "mov rbx, rax");
            asm_add(1, "push rax");
            asm_add(1, "mov rax, rbx");
            asm_add(1, "inc rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            asm_add(1, "pop rax");
            break;
        case UOP_POST_DECR: // x-- 
            // Decrement and return previous value
            if (node->rhs->expr_type != EXPR_VAR) {
                codegen_error("Only variables can be decremented to");
            }
            asm_add_com("; Op: -- (post)");
            asm_add(1, "mov rbx, rax");
            asm_add(1, "push rax");
            asm_add(1, "mov rax, rbx");
            asm_add(1, "dec rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            asm_add(1, "pop rax");
            break;
        case UOP_SIZEOF:
            asm_add_com("; Op: sizeof");
            char buf[64];
            sprintf(buf, "%d", node->rhs->cast_type.bytes);
            asm_add(2, "mov rax, ", buf);
            break;
        case UOP_CAST:
            asm_add_com("; Op: cast");
            gen_asm_unary_op_cast(node, ctx);
            break;
        default:
            codegen_error("Unsupported integer unary operation found!");
            break;
    }
    free(var_sp);
}

// Maybe that is for later when I implement proper expression handling
void gen_asm_binary_op_int(ASTNode* node, AsmContext ctx) {
    gen_asm_setup_short_circuiting(node, &ctx); // AND/OR Short circuiting related

    gen_asm(node->lhs, ctx); // LHS now in RAX

    gen_asm_add_short_circuit_jumps(node, ctx); // AND/OR Short circuiting related

    asm_add(1, "push rax"); // Save RAX
    gen_asm(node->rhs, ctx); // LHS now in RAX
    asm_add(1, "mov rbx, rax"); // Move RHS to RBX
    asm_add(1, "pop rax"); // LHS now in RAX
    // We are now ready for the binary operation
    switch (node->op_type) { // These are all integer operations
        case BOP_ASSIGN: 
            // Rest of assignment is handled after the switch
            asm_add(1, "mov rax, rbx"); // We need the rhs value in rax
            break;
        case BOP_ASSIGN_ADD:
        case BOP_ADD: // Addition
            asm_add_com("; Op: +");
            asm_add(1, "add rax, rbx");
            break;
        case BOP_ASSIGN_SUB: // Assignment subtraction
        case BOP_SUB: // Subtraction
            asm_add_com("; Op: -");
            asm_add(1, "sub rax, rbx");
            break;
        case BOP_ASSIGN_MULT:
        case BOP_MUL: // Multiplication
            asm_add_com("; Op: *");
            asm_add(1, "imul rax, rbx");
            break;
        case BOP_ASSIGN_DIV:
        case BOP_DIV: // Integer division
            asm_add_com("; Op: / (Integer)");
            asm_add(1, "push rdx");
            asm_add(1, "mov rdx, 0"); // Need to reset rdx, won't work otherwise
            asm_add(1, "idiv rbx");
            asm_add(1, "pop rdx");
            break;
        case BOP_ASSIGN_MOD:
        case BOP_MOD: // Modulo
            asm_add_com("; Op: %");
            asm_add(1, "mov rdx, 0");
            asm_add(1, "idiv rbx");
            asm_add(1, "mov rax, rdx"); // Remainder from div is put in rdx
            break;
        // Logical
        case BOP_EQ: // Equals
            asm_add_com("; Op: ==");
            asm_add(1, "cmp rax, rbx");
            asm_add(1, "mov rax, 0");
            asm_add(1, "sete al");
            break;
        case BOP_NEQ: // Not equals
            asm_add_com("; Op: !=");
            asm_add(1, "cmp rax, rbx");
            asm_add(1, "mov rax, 0");
            asm_add(1, "setne al");
            break;
        case BOP_LT: // Less than
            asm_add_com("; Op: <");
            asm_add(1, "cmp rax, rbx");
            asm_add(1, "mov rax, 0");
            asm_add(1, "setl al");
            break;
        case BOP_LTE: // Less than equals
            asm_add_com("; Op: <=");
            asm_add(1, "cmp rax, rbx");
            asm_add(1, "mov rax, 0");
            asm_add(1, "setle al");
            break;
        case BOP_GT: // Greater than
            asm_add_com("; Op: >");
            asm_add(1, "cmp rax, rbx");
            asm_add(1, "mov rax, 0");
            asm_add(1, "setg al");
            break;
        case BOP_GTE: // Greater than equals
            asm_add_com("; Op, >=");
            asm_add(1, "cmp rax, rbx");
            asm_add(1, "mov rax, 0");
            asm_add(1, "setge al");
            break;
        case BOP_AND: // Logical and
            gen_asm_binary_op_and_int(node, ctx);
            break;
        case BOP_OR: // Logical or
            gen_asm_binary_op_or_int(node, ctx);
            break;
        // Bitwise
        case BOP_ASSIGN_BITAND:
        case BOP_BITAND: // Bitwise and
            asm_add_com("; Op: & (BITWISE AND)");
            asm_add(1, "and rax, rbx");
            break;
        case BOP_ASSIGN_BITOR:
        case BOP_BITOR: // Bitwise or
            asm_add_com("; Op: & (BITWISE OR)");
            asm_add(1, "or rax, rbx");
            break;
        case BOP_ASSIGN_BITXOR:
        case BOP_BITXOR: // Bitwise xor
            asm_add_com("; Op: & (BITWISE XOR)");
            asm_add(1, "xor rax, rbx");
            break;
        case BOP_ASSIGN_LEFTSHIFT:
        case BOP_LEFTSHIFT: // Bitwise leftshift
            asm_add_com("; Op: << (BITWISE LEFTSHIFT)");
            asm_add(1, "mov rcx, rbx");
            asm_add(1, "sal rax, cl");
            break;
        case BOP_ASSIGN_RIGHTSHIFT:
        case BOP_RIGHTSHIFT: // Bitwise rightshift
            asm_add_com("; Op: >> (BITWISE RIGHTSHIFT)");
            asm_add(1, "mov rcx, rbx");
            asm_add(1, "sar rax, cl");
            break;
        default:
            codegen_error("Unsupported integer binary operation found!");
            break;
    }
    if (is_binary_operation_assignment(node->op_type)) {
        gen_asm_binary_op_assign_int(node->lhs, ctx);
    }
}

void gen_asm_binary_op_assign_int(ASTNode* node, AsmContext ctx) {
    if (node->expr_type != EXPR_VAR) {
            codegen_error("Only variables can be assigned to");
    } // a = a+1
    char* reg_str = get_reg_width_str(node->var.type.bytes, RAX);
    char* var_sp = var_to_stack_ptr(&node->var);
    asm_add(4, "mov ", var_sp, ", ", reg_str);
    free(var_sp);
}

void gen_asm_binary_op_and_int(ASTNode* node, AsmContext ctx) {
    asm_add_com("; Op: && (AND)");
    asm_add(1, "and rax, rbx");
    asm_add(1, "cmp rax, 0");
    asm_add(1, "mov rax, 0");
    asm_add(1, "setne al");
    if (ctx.and_end_node) { // Add short circuit end jump label
        asm_add(2, ctx.and_short_circuit_label, ": ; Logical short circuit end label");
        free(ctx.and_short_circuit_label);
    }
}

void gen_asm_binary_op_or_int(ASTNode* node, AsmContext ctx) {
    asm_add_com("; Op: || (OR)");
    asm_add(1, "or rax, rbx");
    asm_add(1, "cmp rax, 0");
    asm_add(1, "mov rax, 0");
    asm_add(1, "setne al");
    if (ctx.or_end_node) { // Add short circuit end jump label
        asm_add(2, ctx.or_short_circuit_label, ": ; Logical short circuit end label");
        free(ctx.or_short_circuit_label);
    }
}

// =========== Float operations ===============
// Generate assembly for a float unary op expression node
void gen_asm_unary_op_float(ASTNode* node, AsmContext ctx) {
    gen_asm(node->rhs, ctx); // The value we are acting on is now in RAX
    char* var_sp = var_to_stack_ptr(&node->rhs->var);
    switch (node->op_type) {
        case UOP_SIZEOF:
            asm_add_com("; Op: sizeof");
            char buf[64];
            sprintf(buf, "%d", node->rhs->cast_type.bytes);
            asm_add(2, "mov rax, ", buf);
            break;
        case UOP_CAST:
            asm_add_com("; Op: cast");
            gen_asm_unary_op_cast(node, ctx);
            break;
        default:
            codegen_error("Unsupported float unary operation found!");
            break;
    }
    free(var_sp);
}
// Generate assembly for a binary op expression node
void gen_asm_binary_op_float(ASTNode* node, AsmContext ctx) {
    gen_asm_setup_short_circuiting(node, &ctx); // AND/OR Short circuiting related

    gen_asm(node->lhs, ctx); // LHS now in RAX

    gen_asm_add_short_circuit_jumps(node, ctx); // AND/OR Short circuiting related
    asm_add(1, "movq rax, xmm0");
    asm_add(1, "push rax"); // Save RAX
    gen_asm(node->rhs, ctx);
    asm_add(1, "movq xmm1, xmm0"); // Move RHS to XMM1
    asm_add(1, "pop rax"); // LHS now in RAX
    asm_add(1, "movq xmm0, rax"); // LHS now in XMM0
    // We are now ready for the binary operation
    switch (node->op_type) { // These are all integer operations
        case BOP_ASSIGN: 
            // Rest of assignment is handled after the switch
            asm_add(1, "movq xmm0, xmm1"); // We need the rhs value in rax
            break;
        default:
            codegen_error("Unsupported float binary operation found!");
            break;
    }
    if (is_binary_operation_assignment(node->op_type)) {
        gen_asm_binary_op_assign_float(node->lhs, ctx);
    }
}
// Generate assembly for a binary op assignment expression node
void gen_asm_binary_op_assign_float(ASTNode* node, AsmContext ctx) {
    if (node->expr_type != EXPR_VAR) {
            codegen_error("Only variables can be assigned to");
    }
    char* var_sp = var_to_stack_ptr(&node->var);
    asm_add(3, "movq ", var_sp, ", xmm0");
    free(var_sp);
}

// Short circuiting
void gen_asm_setup_short_circuiting(ASTNode* node, AsmContext* ctx) {
    // Identify first AND and OR nodes, give them labels for short circuiting
    if (ctx->and_end_node) {
        ctx->and_end_node = false;
    }
    if (node->op_type == BOP_AND) { // AND end node found
        if (ctx->and_short_circuit_label == NULL) {
            ctx->and_short_circuit_label = get_next_label_str();
            ctx->and_end_node = true;
        }
    }
    else {
        ctx->and_short_circuit_label = NULL;
    }
    if (ctx->or_end_node) {
        ctx->or_end_node = false;
    }
    if (node->op_type == BOP_OR) { // OR end node found
        if (ctx->or_short_circuit_label == NULL) {
            ctx->or_short_circuit_label = get_next_label_str();
            ctx->or_end_node = true;
        }
    }
    else {
        ctx->or_short_circuit_label = NULL;
    }
}

void gen_asm_add_short_circuit_jumps(ASTNode* node, AsmContext ctx) {
    if (node->op_type == BOP_AND) {
        // Does this ruin rax?
        asm_add(1, "cmp rax, 0");
        asm_add(3, "je ", ctx.and_short_circuit_label, " ; Short circuit AND jump");
    }
    if (node->op_type == BOP_OR) {
        // Does this ruin rax?
        asm_add(1, "cmp rax, 1");
        asm_add(3, "je ", ctx.or_short_circuit_label, " ; Short circuit OR jump");
    }
}

void gen_asm_unary_op_cast(ASTNode* node, AsmContext ctx) {
    VarType to_type = node->cast_type;
    VarType from_type = node->rhs->cast_type;
    // We have value in rax or xmm0
    if (to_type.type == TY_INT && from_type.type == TY_FLOAT) {
        // Float to int
        asm_add_com("; Float to int cast");
        asm_add(1, "cvttsd2si rax, xmm0");
    }
    else if (to_type.type == TY_FLOAT && from_type.type == TY_INT) {
        // Int to float
        asm_add_com("; Int to float cast");
        asm_add(1, "cvtsi2sd xmm0, rax");
    }
    // Int to int, float to float is ignored for now
}