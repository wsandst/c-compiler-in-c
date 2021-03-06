/*
Implementation of code generation functionality related to expressions 
*/
#include "codegen.h"

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node, AsmContext ctx) {
    if (node->expr_type == EXPR_LITERAL) {
        gen_asm_literal(node, ctx);
    }
    else if (node->expr_type == EXPR_VAR) {
        gen_asm_variable(node, ctx);
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

// Generate assembly for accessing a variable
void gen_asm_variable(ASTNode* node, AsmContext ctx) {
    // Access a variable and store it in rax
    char* sp2 = var_to_stack_ptr(&node->var);
    // Handle various variable types
    if (node->var.type.is_const) { // Constant
        asm_addf(&ctx, "mov rax, %s", node->var.const_expr);
    }
    else if (node->var.type.is_struct_member) {
        // Struct member. Lhs must be struct, which means
        // pop rax will give us the lhs memory value
        char* addr_size = bytes_to_addr_width(node->var.type.bytes);
        char* move_instr = get_move_instr_for_var_type(node->var.type);
        int offset = node->var.type.struct_bytes_offset;
        // Save rax for potential deref assignment
        asm_add_com(&ctx, "; Struct member variable access");
        asm_addf(&ctx, "pop rax");
        asm_addf(&ctx, "lea r12, [rax+%d]", offset);
        asm_addf(&ctx, "push rax");
        if (node->var.type.is_array ||
            (node->var.type.type == TY_STRUCT && node->var.type.ptr_level == 0)) {
            // If the member variable is a struct or array, we want the address in rax
            asm_addf(&ctx, "mov rax, r12", offset);
        }
        else if (node->var.type.type == TY_FLOAT) {
            asm_addf(&ctx, "movq xmm0, [r12]");
        }
        else { // Else, get the value
            asm_addf(&ctx, "%s, %s [r12]", move_instr, addr_size);
        }
        free(move_instr);
    }
    else if (node->var.type.is_array ||
             (node->var.type.type == TY_STRUCT && node->var.type.ptr_level == 0)) {
        // We store the address of array/pointers, not the value
        if (node->var.type.is_static) {
            asm_addf(&ctx, "lea rax, [%s.%ds]", node->var.name, node->var.unique_id);
        }
        else if (node->var.is_global) {
            asm_addf(&ctx, "lea rax, [G_%s]", node->var.name);
        }
        else {
            asm_addf(&ctx, "lea rax, [rbp-%d]", node->var.stack_offset);
        }
    }
    else if (node->var.type.type == TY_INT || node->var.type.ptr_level > 0) {
        // Integer/pointer type, store value in rax
        char* move_instr = get_move_instr_for_var_type(node->var.type);
        asm_addf(&ctx, "%s, %s ; var %s", move_instr, sp2, node->var.name);
        free(move_instr);
    }
    else if (node->var.type.type == TY_FLOAT) {
        // Floating point type, store in xmm0
        if (node->var.type.bytes == 4) {
            asm_addf(&ctx, "movd xmm0, %s", sp2);
            asm_addf(&ctx, "cvtss2sd xmm0, xmm0");
        }
        else { // 8 bytes
            asm_addf(&ctx, "movq xmm0, %s", sp2);
        }
    }
    else {
        codegen_error("Unsupported variable type encountered");
    }
    free(sp2);
}

void gen_asm_literal(ASTNode* node, AsmContext ctx) {
    if (node->literal_type == LT_INT) {
        asm_addf(&ctx, "mov rax, %s", node->literal);
    }
    else if (node->literal_type == LT_FLOAT) {
        asm_addf(&ctx, "mov rax, __float64__(%s)", node->literal);
        asm_addf(&ctx, "movq xmm0, rax");
    }
    else if (node->literal_type == LT_STRING) {
        asm_set_indent(&ctx, 0);
        char* label_name = get_next_cstring_label_str(&ctx);
        asm_add_sectionf(&ctx, ctx.asm_rodata_src, "%s: db `%s`, 0", label_name,
                         node->literal);
        asm_set_indent(&ctx, 1);
        asm_addf(&ctx, "lea rax, [%s]", label_name);
    }
    else if (node->literal_type == LT_CHAR) {
        asm_addf(&ctx, "mov rax, `%s`", node->literal);
    }
    else {
        codegen_error("Unsupported literal encountered");
    }
}

void gen_asm_unary_op(ASTNode* node, AsmContext ctx) {
    if (node->cast_type.ptr_level > 0) { // Pointer
        gen_asm_unary_op_ptr(node, ctx);
    }
    else if (node->cast_type.type == TY_INT) {
        gen_asm_unary_op_int(node, ctx);
    }
    else if (node->cast_type.type == TY_FLOAT) {
        gen_asm_unary_op_float(node, ctx);
    }
    else if (node->cast_type.type == TY_STRUCT) {
        gen_asm_unary_op_struct(node, ctx);
    }
    else {
        codegen_error("Invalid cast-type encountered");
    }
}

void gen_asm_binary_op(ASTNode* node, AsmContext ctx) {
    if (node->cast_type.ptr_level > 0) { // Pointer
        gen_asm_binary_op_ptr(node, ctx);
    }
    else if (node->cast_type.type == TY_INT) { // Int
        // We need to perform implicit casting here
        gen_asm_binary_op_int(node, ctx);
    }
    else if (node->cast_type.type == TY_FLOAT) { // Float
        gen_asm_binary_op_float(node, ctx);
    }
    else if (node->cast_type.type == TY_STRUCT) {
        gen_asm_binary_op_struct(node, ctx);
    }
    else {
        codegen_error("Invalid cast-type encountered");
    }
}

// & address operator for
void gen_asm_unary_op_address(ASTNode* node, AsmContext ctx) {
    asm_add_com(&ctx, "; Op: & (address)");
    if (node->expr_type == EXPR_VAR) {
        asm_addf(&ctx, "mov rax, 0");
        asm_addf(&ctx, "lea rax, [rbp-%d]", node->var.stack_offset);
    }
    else if ((node->expr_type == EXPR_UNOP && node->op_type == UOP_DEREF) ||
             (node->expr_type == EXPR_BINOP && node->op_type == BOP_MEMBER)) {
        // The address is in r12
        asm_addf(&ctx, "mov rax, r12");
    }
    else {
        codegen_error("Tried to take address of non-supported operand!");
    }
}

// =============== Integer operations ===============

void gen_asm_unary_op_int(ASTNode* node, AsmContext ctx) {
    gen_asm(node->rhs, ctx); // The value we are acting on is now in RAX
    char* var_sp = var_to_stack_ptr(&node->rhs->var);
    switch (node->op_type) {
        case UOP_NEG: // Negation
            asm_addf(&ctx, "neg rax");
            break;
        case UOP_COMPL: // Complement
            asm_addf(&ctx, "not rax");
            break;
        case UOP_NOT: // Logical not
            asm_addf(&ctx, "cmp rax, 0");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "sete al");
            break;
        // Increment, decrement
        // This is kind of a form of assignment
        case UOP_PRE_INCR: // ++x
            // Increment and return incremented value
            asm_add_com(&ctx, "; Op: ++ (pre)");
            asm_addf(&ctx, "inc rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            break;
        case UOP_PRE_DECR: // --x
            // Decrement and return incremented value
            asm_add_com(&ctx, "; Op: -- (pre)");
            asm_addf(&ctx, "dec rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            break;
        case UOP_POST_INCR: // x++
            // Increment and return previous value
            asm_add_com(&ctx, "; Op: ++ (post)");
            asm_addf(&ctx, "mov rbx, rax");
            asm_addf(&ctx, "push rax");
            asm_addf(&ctx, "mov rax, rbx");
            asm_addf(&ctx, "inc rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            asm_addf(&ctx, "pop rax");
            break;
        case UOP_POST_DECR: // x--
            // Decrement and return previous value
            asm_add_com(&ctx, "; Op: -- (post)");
            asm_addf(&ctx, "mov rbx, rax");
            asm_addf(&ctx, "push rax");
            asm_addf(&ctx, "mov rax, rbx");
            asm_addf(&ctx, "dec rax");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            asm_addf(&ctx, "pop rax");
            break;
        case UOP_SIZEOF:
            asm_add_com(&ctx, "; Op: sizeof");
            if (node->rhs->cast_type.is_array) {
                asm_addf(&ctx, "mov rax, %d",
                         node->rhs->cast_type.array_size *
                             node->rhs->cast_type.ptr_value_bytes);
            }
            else if (node->rhs->cast_type.type == TY_STRUCT &&
                     node->rhs->cast_type.ptr_level == 0) {
                asm_addf(&ctx, "mov rax, %d", node->rhs->var.struct_type.struct_type.bytes);
            }
            else {
                asm_addf(&ctx, "mov rax, %d", node->rhs->cast_type.bytes);
            }
            break;
        case UOP_CAST:
            asm_add_com(&ctx, "; Op: cast");
            gen_asm_unary_op_cast(ctx, node->cast_type, node->rhs->cast_type);
            break;
        case UOP_DEREF: { // Deref from int pointer
            asm_add_com(&ctx, "; Op: * (deref)");
            char* addr_size = bytes_to_addr_width(node->cast_type.bytes);
            char* move_instr = get_move_instr_for_var_type(node->cast_type);
            asm_addf(&ctx, "mov r12, rax"); // Save rax for potential deref assignment
            asm_addf(&ctx, "%s, %s [rax]", move_instr, addr_size);
            free(move_instr);
            break;
        }
        default:
            codegen_error("Unsupported integer unary operation found!");
            break;
    }
    free(var_sp);
}

// If we perform assignment, and lhs is a deref op, we want to assign to the var, not the.
// In the parser, when we encounter a deref, we need to copy the variable up a step

// Current

// Maybe that is for later when I implement proper expression handling
void gen_asm_binary_op_int(ASTNode* node, AsmContext ctx) {
    gen_asm_setup_short_circuiting(node, &ctx); // AND/OR Short circuiting related

    gen_asm(node->lhs, ctx); // LHS now in RAX
    if (is_binary_operation_assignment(node->op_type)) {
        // Address of lvalue is in r12
        // lvalues are only used in assignment, thus we need to save r12
        // incase rhs contains another lvalue
        asm_addf(&ctx, "push r12");
    }

    gen_asm_add_short_circuit_jumps(node, ctx); // AND/OR Short circuiting related

    asm_addf(&ctx, "push rax"); // Save RAX
    gen_asm(node->rhs, ctx); // LHS now in RAX
    // Check if we need to cast rhs
    gen_asm_unary_op_cast(ctx, node->cast_type, node->rhs->cast_type);
    asm_addf(&ctx, "mov rbx, rax"); // Move RHS to RBX
    asm_addf(&ctx, "pop rax"); // LHS now in RAX
    // We are now ready for the binary operation
    switch (node->op_type) { // These are all integer operations
        case BOP_ASSIGN:
            // Rest of assignment is handled after the switch
            asm_add_com(&ctx, "; Op: =");
            asm_addf(&ctx, "mov rax, rbx"); // We need the rhs value in rax
            break;
        case BOP_ASSIGN_ADD:
        case BOP_ADD: // Addition
            asm_add_com(&ctx, "; Op: +");
            asm_addf(&ctx, "add rax, rbx");
            break;
        case BOP_ASSIGN_SUB: // Assignment subtraction
        case BOP_SUB: // Subtraction
            asm_add_com(&ctx, "; Op: -");
            asm_addf(&ctx, "sub rax, rbx");
            break;
        case BOP_ASSIGN_MULT:
        case BOP_MUL: // Multiplication
            asm_add_com(&ctx, "; Op: *");
            asm_addf(&ctx, "imul rax, rbx");
            break;
        case BOP_ASSIGN_DIV:
        case BOP_DIV: // Integer division
            asm_add_com(&ctx, "; Op: / (Integer)");
            asm_addf(&ctx, "push rdx");
            asm_addf(&ctx, "mov rdx, 0"); // Need to reset rdx, won't work otherwise
            asm_addf(&ctx, "idiv rbx");
            asm_addf(&ctx, "pop rdx");
            break;
        case BOP_ASSIGN_MOD:
        case BOP_MOD: // Modulo
            asm_add_com(&ctx, "; Op: %");
            asm_addf(&ctx, "mov rdx, 0");
            asm_addf(&ctx, "idiv rbx");
            asm_addf(&ctx, "mov rax, rdx"); // Remainder from div is put in rdx
            break;
        // Logical
        case BOP_EQ: // Equals
            asm_add_com(&ctx, "; Op: ==");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "sete al");
            break;
        case BOP_NEQ: // Not equals
            asm_add_com(&ctx, "; Op: !=");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setne al");
            break;
        case BOP_LT: // Less than
            asm_add_com(&ctx, "; Op: <");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setl al");
            break;
        case BOP_LTE: // Less than equals
            asm_add_com(&ctx, "; Op: <=");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setle al");
            break;
        case BOP_GT: // Greater than
            asm_add_com(&ctx, "; Op: >");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setg al");
            break;
        case BOP_GTE: // Greater than equals
            asm_add_com(&ctx, "; Op, >=");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setge al");
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
            asm_add_com(&ctx, "; Op: & (BITWISE AND)");
            asm_addf(&ctx, "and rax, rbx");
            break;
        case BOP_ASSIGN_BITOR:
        case BOP_BITOR: // Bitwise or
            asm_add_com(&ctx, "; Op: & (BITWISE OR)");
            asm_addf(&ctx, "or rax, rbx");
            break;
        case BOP_ASSIGN_BITXOR:
        case BOP_BITXOR: // Bitwise xor
            asm_add_com(&ctx, "; Op: & (BITWISE XOR)");
            asm_addf(&ctx, "xor rax, rbx");
            break;
        case BOP_ASSIGN_LEFTSHIFT:
        case BOP_LEFTSHIFT: // Bitwise leftshift
            asm_add_com(&ctx, "; Op: << (BITWISE LEFTSHIFT)");
            asm_addf(&ctx, "mov rcx, rbx");
            asm_addf(&ctx, "sal rax, cl");
            break;
        case BOP_ASSIGN_RIGHTSHIFT:
        case BOP_RIGHTSHIFT: // Bitwise rightshift
            asm_add_com(&ctx, "; Op: >> (BITWISE RIGHTSHIFT)");
            asm_addf(&ctx, "mov rcx, rbx");
            asm_addf(&ctx, "sar rax, cl");
            break;
        case BOP_MEMBER: // Struct to int member
            asm_add_com(&ctx, "; Op: struct member");
            asm_addf(&ctx, "mov rax, rbx");
            break;
        default:
            codegen_error("Unsupported integer binary operation found!");
            break;
    }
    if (is_binary_operation_assignment(node->op_type)) {
        asm_addf(&ctx, "pop r12"); // Restore r12 lvalue address
        gen_asm_binary_op_assign_int(node->lhs, ctx);
    }
}

void gen_asm_binary_op_assign_int(ASTNode* node, AsmContext ctx) {
    if (node->expr_type == EXPR_VAR) {
        char* reg_str = get_reg_width_str(node->var.type.bytes, RAX);
        char* var_sp = var_to_stack_ptr(&node->var);
        asm_addf(&ctx, "mov %s, %s", var_sp, reg_str);
        free(var_sp);
    }
    else if (node->expr_type == EXPR_UNOP && node->op_type == UOP_DEREF) {
        node->var.type.bytes = node->var.type.ptr_value_bytes;
        char* reg_str = get_reg_width_str(node->cast_type.bytes, RAX);
        char* addr_size_str = bytes_to_addr_width(node->cast_type.bytes);
        asm_addf(&ctx, "mov %s [r12], %s", addr_size_str, reg_str);
    }
    else if (node->expr_type == EXPR_BINOP && node->op_type == BOP_MEMBER) {
        char* reg_str = get_reg_width_str(node->cast_type.bytes, RAX);
        char* addr_size_str = bytes_to_addr_width(node->cast_type.bytes);
        asm_addf(&ctx, "mov %s [r12], %s", addr_size_str, reg_str);
    }
    else {
        codegen_error("Only variables can be assigned to");
    }
}

void gen_asm_binary_op_and_int(ASTNode* node, AsmContext ctx) {
    asm_add_com(&ctx, "; Op: && (AND)");

    // rax != 0
    asm_addf(&ctx, "cmp rax, 0");
    asm_addf(&ctx, "setne al");

    // rbx != 0
    asm_addf(&ctx, "cmp rbx, 0");
    asm_addf(&ctx, "setne bl");

    // rax & rbx
    asm_addf(&ctx, "and al, bl");
    asm_addf(&ctx, "mov rax, 0");
    asm_addf(&ctx, "setne al");
    if (ctx.and_end_node) { // Add short circuit end jump label
        asm_addf(&ctx, "%s: ; Logical short circuit end label",
                 ctx.and_short_circuit_label);
        free(ctx.and_short_circuit_label);
    }
}

void gen_asm_binary_op_or_int(ASTNode* node, AsmContext ctx) {
    asm_add_com(&ctx, "; Op: || (OR)");
    asm_addf(&ctx, "or rax, rbx");
    asm_addf(&ctx, "cmp rax, 0");
    asm_addf(&ctx, "mov rax, 0");
    asm_addf(&ctx, "setne al");
    if (ctx.or_end_node) { // Add short circuit end jump label
        asm_addf(&ctx, "%s: ; Logical short circuit end label", ctx.or_short_circuit_label);
        free(ctx.or_short_circuit_label);
    }
}

// =========== Float operations ===============
// Generate assembly for a float unary op expression node
void gen_asm_unary_op_float(ASTNode* node, AsmContext ctx) {
    gen_asm(node->rhs, ctx); // The value we are acting on is now in RAX
    char* var_sp = var_to_stack_ptr(&node->rhs->var);
    switch (node->op_type) {
        case UOP_NEG: // Negation
            // Move into integer reg, flip first bit with xor
            asm_addf(&ctx, "movq rbx, xmm0");
            asm_addf(&ctx, "mov rax, 0x8000000000000000");
            asm_addf(&ctx, "xor rax, rbx");
            asm_addf(&ctx, "movq xmm0, rax");
            break;
        case UOP_SIZEOF:
            asm_add_com(&ctx, "; Op: sizeof");
            asm_addf(&ctx, "mov rax, %s", node->rhs->cast_type.bytes);
            break;
        case UOP_CAST:
            asm_add_com(&ctx, "; Op: cast");
            gen_asm_unary_op_cast(ctx, node->cast_type, node->rhs->cast_type);
            break;
        case UOP_DEREF: { // Deref from int pointer
            asm_add_com(&ctx, "; fOp: * (deref)");
            char* addr_size = bytes_to_addr_width(node->cast_type.bytes);
            char* move_instr = get_move_instr_for_var_type(node->cast_type);
            asm_addf(&ctx, "mov r12, rax"); // Save rax for potential deref assignment
            asm_addf(&ctx, "%s, %s [rax]", move_instr, addr_size);
            if (node->cast_type.bytes == 4) {
                asm_addf(&ctx, "movd xmm0, eax");
                asm_addf(&ctx, "cvtss2sd xmm0, xmm0");
            }
            else {
                asm_addf(&ctx, "movq xmm0, rax");
            }
            free(move_instr);
            break;
        }
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
    if (is_binary_operation_assignment(node->op_type)) {
        // Address of lvalue is in r12
        // lvalues are only used in assignment, thus we need to save r12
        // incase rhs contains another lvalue
        asm_addf(&ctx, "push r12");
    }
    // Check if we need to cast lhs (lhs is int)
    gen_asm_unary_op_cast(ctx, node->cast_type, node->lhs->cast_type);

    gen_asm_add_short_circuit_jumps(node, ctx); // AND/OR Short circuiting related

    // Save xmm0 in rax. In a few cases we don't want to do this, check for that
    if (!(node->lhs->expr_type == EXPR_VAR && node->lhs->var.type.type == TY_STRUCT)) {
        asm_addf(&ctx, "movq rax, xmm0");
    }
    asm_addf(&ctx, "push rax"); // Save RAX
    gen_asm(node->rhs, ctx);
    // Check if we need to cast rhs (rhs is int)
    gen_asm_unary_op_cast(ctx, node->cast_type, node->rhs->cast_type);
    asm_addf(&ctx, "movq xmm1, xmm0"); // Move RHS to XMM1
    asm_addf(&ctx, "pop rax"); // LHS now in RAX
    asm_addf(&ctx, "movq xmm0, rax"); // LHS now in XMM0
    // We are now ready for the binary operation
    switch (node->op_type) { // These are all integer operations
        case BOP_ASSIGN:
            // Rest of assignment is handled after the switch
            asm_add_com(&ctx, "; fOp: =");
            asm_addf(&ctx, "movq xmm0, xmm1"); // We need the rhs value in rax
            break;
        case BOP_ASSIGN_ADD:
        case BOP_ADD: // Addition
            asm_add_com(&ctx, "; fOp: +");
            asm_addf(&ctx, "addsd xmm0, xmm1");
            break;
        case BOP_ASSIGN_SUB:
        case BOP_SUB: // Subtraction
            asm_add_com(&ctx, "; fOp: -");
            asm_addf(&ctx, "subsd xmm0, xmm1");
            break;
        case BOP_ASSIGN_MULT:
        case BOP_MUL: // Multiplication
            asm_add_com(&ctx, "; fOp: *");
            asm_addf(&ctx, "mulsd xmm0, xmm1");
            break;
        case BOP_ASSIGN_DIV:
        case BOP_DIV: // Division
            asm_add_com(&ctx, "; fOp: / (Integer)");
            asm_addf(&ctx, "divsd xmm0, xmm1");
            break;
        case BOP_LT:
            asm_add_com(&ctx, "; fOp: < (Integer)");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "comisd xmm0, xmm1");
            asm_addf(&ctx, "setb al");
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
            break;
        case BOP_LTE:
            asm_add_com(&ctx, "; fOp: <= (Integer)");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "comisd xmm0, xmm1");
            asm_addf(&ctx, "setbe al");
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
            break;
        case BOP_GT:
            asm_add_com(&ctx, "; fOp: > (Integer)");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "comisd xmm0, xmm1");
            asm_addf(&ctx, "seta al");
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
            break;
        case BOP_GTE:
            asm_add_com(&ctx, "; fOp: >= (Integer)");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "comisd xmm0, xmm1");
            asm_addf(&ctx, "setae al");
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
            break;
        case BOP_EQ:
            asm_add_com(&ctx, "; fOp: == (Integer)");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "comisd xmm0, xmm1");
            asm_addf(&ctx, "sete al");
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
            break;
        case BOP_NEQ:
            asm_add_com(&ctx, "; fOp: != (Integer)");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "comisd xmm0, xmm1");
            asm_addf(&ctx, "setne al");
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
            break;
        case BOP_MEMBER:
            asm_add_com(&ctx, "; fOp: struct member");
            asm_addf(&ctx, "movq xmm0, xmm1");
            break;
        default:
            codegen_error("Unsupported float binary operation found!");
            break;
    }
    if (is_binary_operation_assignment(node->op_type)) {
        asm_addf(&ctx, "pop r12"); // Restore r12 lvalue address
        gen_asm_binary_op_assign_float(node->lhs, ctx);
    }
}
// Generate assembly for a binary op assignment expression node
void gen_asm_binary_op_assign_float(ASTNode* node, AsmContext ctx) {
    char* move_instr = get_float_move_for_byte_size(node->cast_type.bytes);
    if (node->cast_type.bytes == 4) { // Convert to 32 bit float if assigning to 32 bit
        asm_addf(&ctx, "cvtsd2ss xmm0, xmm0");
    }
    if (node->expr_type == EXPR_VAR) {
        char* var_sp = var_to_stack_ptr(&node->var);
        asm_addf(&ctx, "%s %s, xmm0", move_instr, var_sp);
        free(var_sp);
    }
    else if (node->expr_type == EXPR_UNOP && node->op_type == UOP_DEREF) {
        asm_addf(&ctx, "%s [r12], xmm0", move_instr);
    }
    else if (node->expr_type == EXPR_BINOP && node->op_type == BOP_MEMBER) {
        asm_addf(&ctx, "%s [r12], xmm0", move_instr);
    }
    else {
        codegen_error("Only variables can be assigned to");
    }
}

// =============== Pointer operations ===============
// Generate assembly for a float unary op expression node
void gen_asm_unary_op_ptr(ASTNode* node, AsmContext ctx) {
    gen_asm(node->rhs, ctx); // The value we are acting on is now in RAX
    char* var_sp = var_to_stack_ptr(&node->rhs->var);
    switch (node->op_type) {
        case UOP_ADDR:
            gen_asm_unary_op_address(node->rhs, ctx);
            break;
        case UOP_DEREF: // Deref from pointer to pointer
            asm_add_com(&ctx, "; Op: * (deref)");
            asm_addf(&ctx, "mov r12, rax");
            asm_addf(&ctx, "mov rax, qword [rax]");
            break;
        case UOP_CAST:
            asm_add_com(&ctx, "; Op: cast");
            gen_asm_unary_op_cast(ctx, node->cast_type, node->rhs->cast_type);
            break;
        // Increment, decrement
        // This is kind of a form of assignment
        case UOP_PRE_INCR: // ++x
            asm_add_com(&ctx, "; pOp: ++ (pre)");
            asm_addf(&ctx, "mov rbx, 1");
            gen_asm_binary_op_load_ptr_size(node, ctx);
            asm_addf(&ctx, "add rax, rbx");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            break;
        case UOP_PRE_DECR: // --x
            asm_add_com(&ctx, "; pOp: -- (pre)");
            asm_addf(&ctx, "mov rbx, 1");
            gen_asm_binary_op_load_ptr_size(node, ctx);
            asm_addf(&ctx, "sub rax, rbx");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            break;
        case UOP_POST_INCR: // x++
            asm_add_com(&ctx, "; pOp: ++ (post)");
            asm_addf(&ctx, "mov rbx, rax");
            asm_addf(&ctx, "push rax");
            asm_addf(&ctx, "mov rax, rbx");
            asm_addf(&ctx, "mov rbx, 1");
            gen_asm_binary_op_load_ptr_size(node, ctx);
            asm_addf(&ctx, "add rax, rbx");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            asm_addf(&ctx, "pop rax");
            break;
        case UOP_POST_DECR: // x--
            asm_add_com(&ctx, "; pOp: -- (post)");
            asm_addf(&ctx, "mov rbx, rax");
            asm_addf(&ctx, "push rax");
            asm_addf(&ctx, "mov rax, rbx");
            asm_addf(&ctx, "mov rbx, 1");
            gen_asm_binary_op_load_ptr_size(node, ctx);
            asm_addf(&ctx, "sub rax, rbx");
            gen_asm_binary_op_assign_int(node->rhs, ctx);
            asm_addf(&ctx, "pop rax");
            break;
        case UOP_NOT: // Logical not
            asm_addf(&ctx, "cmp rax, 0");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "sete al");
            break;
        default:
            codegen_error("Unsupported pointer unary operation encountered!");
            break;
    }
    free(var_sp);
}

// Generate assembly for a binary op expression node
void gen_asm_binary_op_ptr(ASTNode* node, AsmContext ctx) {
    gen_asm_setup_short_circuiting(node, &ctx); // AND/OR Short circuiting related

    gen_asm(node->lhs, ctx); // LHS now in RAX
    if (is_binary_operation_assignment(node->op_type)) {
        // Address of lvalue is in r12
        // lvalues are only used in assignment, thus we need to save r12
        // incase rhs contains another lvalue
        asm_addf(&ctx, "push r12");
    }
    // Check if we need to cast lhs
    gen_asm_unary_op_cast(ctx, node->cast_type, node->lhs->cast_type);
    gen_asm_add_short_circuit_jumps(node, ctx); // AND/OR Short circuiting related

    asm_addf(&ctx, "push rax"); // Save RAX
    gen_asm(node->rhs, ctx); // LHS now in RAX
    asm_addf(&ctx, "mov rbx, rax"); // Move RHS to RBX
    // Check if we need to cast rhs
    gen_asm_unary_op_cast(ctx, node->cast_type, node->rhs->cast_type);
    asm_addf(&ctx, "pop rax"); // LHS now in RAX
    // We are now ready for the binary operation
    switch (node->op_type) { // These are all integer operations
        case BOP_ASSIGN:
            // Rest of assignment is handled after the switch
            asm_add_com(&ctx, "; pOp: =");
            asm_addf(&ctx, "mov rax, rbx"); // We need the rhs value in rax
            break;
        case BOP_ASSIGN_ADD:
        case BOP_ADD: // Addition
            asm_add_com(&ctx, "; pOp: +");
            gen_asm_binary_op_load_ptr_size(node, ctx);
            asm_addf(&ctx, "add rax, rbx");
            break;
        case BOP_ASSIGN_SUB: // Assignment subtraction
        case BOP_SUB: // Subtraction
            asm_add_com(&ctx, "; pOp: -");
            gen_asm_binary_op_load_ptr_size(node, ctx);
            asm_addf(&ctx, "sub rax, rbx");
            break;
        // Logical
        case BOP_EQ: // Equals
            asm_add_com(&ctx, "; Op: ==");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "sete al");
            break;
        case BOP_NEQ: // Not equals
            asm_add_com(&ctx, "; pOp: !=");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setne al");
            break;
        case BOP_LT: // Less than
            asm_add_com(&ctx, "; pOp: <");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setl al");
            break;
        case BOP_LTE: // Less than equals
            asm_add_com(&ctx, "; pOp: <=");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setle al");
            break;
        case BOP_GT: // Greater than
            asm_add_com(&ctx, "; pOp: >");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setg al");
            break;
        case BOP_GTE: // Greater than equals
            asm_add_com(&ctx, "; pOp, >=");
            asm_addf(&ctx, "cmp rax, rbx");
            asm_addf(&ctx, "mov rax, 0");
            asm_addf(&ctx, "setge al");
            break;
        case BOP_AND: // Logical and
            gen_asm_binary_op_and_int(node, ctx);
            break;
        case BOP_OR: // Logical or
            gen_asm_binary_op_or_int(node, ctx);
            break;
        case BOP_MEMBER: // Struct to pointer member
            asm_add_com(&ctx, "; pOp: struct member");
            asm_addf(&ctx, "mov rax, rbx");
            break;
        default:
            codegen_error("Unsupported pointer binary operation encountered!");
            break;
    }
    if (is_binary_operation_assignment(node->op_type)) {
        asm_addf(&ctx, "pop r12"); // Restore r12 lvalue address
        gen_asm_binary_op_assign_int(node->lhs, ctx);
    }
}

void gen_asm_binary_op_load_ptr_size(ASTNode* node, AsmContext ctx) {
    // Multiply rbx with pointer size
    // We need to check for type here. Only multiply if int
    int bytes = get_deref_var_type(node->cast_type).bytes;
    asm_addf(&ctx, "imul rbx, %d", bytes);
}

// Generate assembly for a struct unary op expression node
void gen_asm_unary_op_struct(ASTNode* node, AsmContext ctx) {
    // Only deref from pointer into struct and sizeof allowed
    gen_asm(node->rhs, ctx); // The value we are acting on is now in RAX
    switch (node->op_type) {
        case UOP_SIZEOF:
            asm_add_com(&ctx, "; sOp: sizeof");
            asm_addf(&ctx, "mov rax, %d", node->rhs->cast_type.bytes);
            break;
        case UOP_DEREF: { // Deref from struct pointer
            asm_add_com(&ctx, "; sOp: * (deref)");
            asm_addf(&ctx, "mov r12, rax"); // Save rax for potential deref assignment
            // We want to keep the pointer in RAX
            break;
        }
        default:
            codegen_error("Unsupported integer unary operation found!");
            break;
    }
}
// Generate assembly for a struct binary op expression node
void gen_asm_binary_op_struct(ASTNode* node, AsmContext ctx) {
    gen_asm(node->lhs, ctx); // LHS now in RAX
    if (node->op_type == BOP_ASSIGN) {
        // Deref address is in r12, we need to save it incase rhs is deref
        asm_addf(&ctx, "push r12");
    }

    asm_addf(&ctx, "push rax"); // Save RAX
    gen_asm(node->rhs, ctx); // LHS now in RAX
    asm_addf(&ctx, "mov rbx, rax"); // Move RHS to RBX
    asm_addf(&ctx, "pop rax"); // LHS now in RAX
    // We are now ready for the binary operation
    switch (node->op_type) { // These are all integer operations
        case BOP_ASSIGN:
            // Rest of assignment is handled after the switch
            asm_add_com(&ctx, "; pOp: =");
            //asm_addf(&ctx, "mov rax, rbx"); // We need the rhs value in rax
            break;
        case BOP_MEMBER: {
            // Address is in rbx
            asm_addf(&ctx, "mov rax, rbx");
            break;
        }
        default:
            codegen_error("Unsupported pointer binary operation encountered!");
            break;
    }
    if (node->op_type == BOP_ASSIGN) {
        asm_addf(&ctx, "pop r12");
    }
    if (is_binary_operation_assignment(node->op_type)) {
        gen_asm_binary_op_assign_struct(node, ctx);
    }
}

// Generate assembly for a struct binary op assignment expression node
void gen_asm_binary_op_assign_struct(ASTNode* node, AsmContext ctx) {
    // Perform a memcpy from address in rbx to address in rax
    if (node->rhs->var.struct_type.struct_type.bytes != node->lhs->cast_type.bytes) {
        codegen_error("Attempted to assign a struct to a struct of different size!");
    }
    // memcpy: rdi: dest_ptr, rsi: src_ptr, rdx: size_t (bytes)
    asm_addf(&ctx, "mov rdi, rax");
    asm_addf(&ctx, "mov rsi, rbx");
    asm_addf(&ctx, "mov rdx, %d", node->rhs->var.struct_type.struct_type.bytes);
    asm_addf(&ctx, "call memcpy");
}

// Short circuiting
void gen_asm_setup_short_circuiting(ASTNode* node, AsmContext* ctx) {
    // Identify first AND and OR nodes, give them labels for short circuiting
    if (ctx->and_end_node) {
        ctx->and_end_node = false;
    }
    if (node->op_type == BOP_AND) { // AND end node found
        if (ctx->and_short_circuit_label == NULL) {
            ctx->and_short_circuit_label = str_copy(get_next_label_str(ctx));
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
            ctx->or_short_circuit_label = str_copy(get_next_label_str(ctx));
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
        asm_addf(&ctx, "cmp rax, 0");
        asm_addf(&ctx, "je %s  ; Short circuit AND jump", ctx.and_short_circuit_label);
    }
    if (node->op_type == BOP_OR) {
        // Does this ruin rax?
        asm_addf(&ctx, "cmp rax, 1");
        asm_addf(&ctx, "je %s ; Short circuit OR jump", ctx.or_short_circuit_label);
    }
}

void gen_asm_unary_op_cast(AsmContext ctx, VarType to_type, VarType from_type) {
    // We have value in rax or xmm0
    if (to_type.ptr_level > 0 && from_type.ptr_level > 0) {
        // Pointer to pointer
        return; // No need to do anything
    }
    else if (to_type.type == TY_INT && from_type.type == TY_FLOAT) {
        // Float to int
        asm_add_com(&ctx, "; Float to int cast");
        if (from_type.bytes == 4) {
            asm_addf(&ctx, "cvttsd2si eax, xmm0");
        }
        else { // 8 bytes
            asm_addf(&ctx, "cvttsd2si rax, xmm0");
        }
    }
    else if (to_type.type == TY_FLOAT && from_type.type == TY_INT) {
        // Int to float
        asm_add_com(&ctx, "; Int to float cast");
        if (to_type.bytes == 4) {
            asm_addf(&ctx, "cvtsi2sd xmm0, eax");
        }
        else { // 8 bytes
            asm_addf(&ctx, "cvtsi2sd xmm0, rax");
        }
    }
    else if (to_type.type == TY_INT && from_type.ptr_level > 0) {
        // Pointer to int
        return;
    }
    else if (to_type.ptr_level > 0 && from_type.type == TY_INT) {
        // Int to pointer
        return;
    }
    else if (to_type.type == TY_INT && from_type.type == TY_INT) {
        return; // No need to do anything
    }
    else if (to_type.type == TY_FLOAT && from_type.type == TY_FLOAT) {
        return; // No need to do anything
    }
    else if (to_type.type == TY_VOID) {
        return;
    }
    else if (from_type.type == TY_STRUCT) {
        return;
    }
    else {
        codegen_error("Unsupported cast attempted!");
    }
    // Int to int, float to float is ignored for now
}

VarType promote_type(VarType type) {
    if (type.type == TY_FLOAT && type.ptr_level == 0) {
        type.bytes = 8;
    }
    return type;
}