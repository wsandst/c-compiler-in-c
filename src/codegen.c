#include "codegen.h"

// Toggle for including comments in the asm output code
const bool INCLUDE_COMMENTS = true;

StrVector asm_src;
char* asm_indent_str;
int label_count = 1;
int indent_level = 0;

static char *rax_modifier_strs[4] = {"al", "ax", "eax", "rax"};
static char *rbx_modifier_strs[4] = {"bl", "bx", "ebx", "rbx"};
static char *rcx_modifier_strs[4] = {"cl", "cx", "ecx", "rcx"};
static char *rdx_modifier_strs[4] = {"dl", "dx", "edx", "rdx"};
static char *rsi_modifier_strs[4] = {"sil", "si", "esi", "rsi"};
static char *rdi_modifier_strs[4] = {"dil", "di", "edi", "rdi"};
static char *r8_modifier_strs[4] =  {"r8b", "r8w", "r8d", "r8"};
static char *r9_modifier_strs[4] =  {"r9b", "r9w", "r9d", "r9"};

static char** register_enum_to_modifier_strs[14] = {
    rax_modifier_strs, rbx_modifier_strs, rcx_modifier_strs, rdx_modifier_strs,
    rsi_modifier_strs, rdi_modifier_strs, r8_modifier_strs, r9_modifier_strs, 
    NULL, NULL, NULL, NULL, NULL, NULL
};

void asm_add_single(StrVector* src, char* str) {
    str_vec_push(src, str);
}

void asm_add(int n, ...) {
    asm_add_newline();
    char* str;
    va_list vl;
    va_start(vl, n);
    for (int i = 0; i < n; i++)
    {
        str = va_arg(vl, char*);
        asm_add_single(&asm_src, str);
    }
    va_end(vl);
}

void asm_add_com(char* comment) {
    if (INCLUDE_COMMENTS) {
        asm_add_newline();
        asm_add_single(&asm_src, comment);
    }
}

void asm_add_newline() {
    char buf[64];
    snprintf(buf, 63, "\n%s", asm_indent_str);
    asm_add_single(&asm_src, buf);
}

void asm_update_indent() {
    free(asm_indent_str);
    asm_indent_str = str_multiply("    ", indent_level);
}

void asm_set_indent(int indent) {
    indent_level = indent;
    asm_update_indent();
}

void asm_add_indent(int amount) {
    indent_level += amount;
    asm_update_indent();
}

void codegen_error(char* error_message) {
    fprintf(stderr, "Codegen error: %s\n", error_message);
    // We are not manually freeing the memory here, 
    // but as the program is exiting it is fine
    exit(1); 
}

char* offset_to_stack_ptr(int offset, char* prefix) {
    char buf[64];
    snprintf(buf, 63, "%s [rbp-%i]", prefix, offset);
    return str_copy(buf);
}

char* var_to_stack_ptr(Variable* var) {
    if (!var->is_global) {
        switch (var->type.bytes) {
            case 8:
                return offset_to_stack_ptr(var->stack_offset, "qword");
            case 4:
                return offset_to_stack_ptr(var->stack_offset, "dword");
            case 2:
                return offset_to_stack_ptr(var->stack_offset, "word");
            case 1:
                return offset_to_stack_ptr(var->stack_offset, "byte");
            default:
                return str_copy("error");
        }
        
    }
    else { // Global variable
        char buf[64];
        snprintf(buf, 63, "[%s]", var->name);
        return str_copy(buf);
    }
    return str_copy("error");
}

char* get_reg_width_str(int size, RegisterEnum reg) {
    int index;
    switch (size) {
        case 8:
            index = 3;
            break;
        default:
            index = size / 2;
    }
    return register_enum_to_modifier_strs[reg][index];
}

char* get_label_str(int label) {
    char result[64];
    sprintf(result, ".L%d", label);
    return str_copy(result);
}

char* get_case_label_str(int label, char* value) {
    char result[64];
    sprintf(result, ".LC%d_%s", label, value);
    return str_copy(result);
}

char* get_next_label_str() {
    label_count++;
    return get_label_str(label_count);
}

char* generate_assembly(AST* ast, SymbolTable* symbols) {
    asm_src = str_vec_new(16);
    asm_indent_str = calloc(1, sizeof(char));

    // Setup globals/functions
    asm_set_indent(0);
    gen_asm_symbols(symbols);

    asm_add(1, "section .text");
    // Setup context object
    AsmContext ctx;
    ctx.last_start_label = NULL;
    ctx.last_end_label = NULL;
    ctx.and_short_circuit_label = NULL;
    ctx.or_short_circuit_label = NULL;
    ctx.and_end_node = false;
    ctx.or_end_node = false;

    gen_asm(ast->program, ctx);
    asm_add_newline();

    char* asm_src_str = str_vec_join(&asm_src);
    str_vec_free(&asm_src);
    free(asm_indent_str);
    return asm_src_str;
}

void gen_asm_symbols(SymbolTable* symbols) {
    // Setup function globals
    asm_add_com("; External or global functions");
    for (size_t i = 0; i < symbols->func_count; i++) {
        Function func = symbols->funcs[i];
        if (func.is_defined) {
            asm_add(2, "global ", func.name);
        }
        else {
            // Undefined functions are set to extern for linker
            asm_add(2, "extern ", func.name); 
        }
    }   
    asm_add_newline();

    // The variables in this scope are always global
    if (symbols->var_count == 0) { // No need if there are no globals
        return;
    }
    // .data section, globals with constants
    asm_add(1, "section .data");
    asm_set_indent(1);
    int undefined_count = 0;
    for (size_t i = 0; i < symbols->var_count; i++) {
        Variable var = symbols->vars[i];
        if (!var.is_undefined) {
            asm_add(2, "global ", var.name);
            asm_add(3, var.name, ": dq ", var.const_expr);
        }
        else {
            undefined_count++;
        }
    }   
    // .bss section, uninitialized globals
    if (undefined_count) { // Only add .bss if necessary
        asm_set_indent(0);
        asm_add_newline();
        asm_add(1, "section .bss");
        asm_set_indent(1);
        for (size_t i = 0; i < symbols->var_count; i++) {
            Variable var = symbols->vars[i];
            if (var.is_undefined) {
                asm_add(2, "global ", var.name);
                asm_add(2, var.name, ": resq 1 ");
            }
        }   
    }
    asm_set_indent(0);
    asm_add_newline();
}

void gen_asm(ASTNode* node, AsmContext ctx) {
    switch (node->type) {
        case AST_PROGRAM:
            gen_asm(node->body, ctx);
            break;
        case AST_FUNC:
            gen_asm_func(node, ctx);
            break;
        case AST_EXPR:
            gen_asm_expr(node, ctx);
            break;
        case AST_SCOPE:
            // Blocks/scopes are a virtual construct, does not exist in the assembly
            gen_asm(node->body, ctx);
            gen_asm(node->next, ctx);
            break;
        case AST_IF: // If conditional
            gen_asm_if(node, ctx);
            break;
        case AST_LOOP: // For and while loops
            gen_asm_loop(node, ctx);
            break;
        case AST_DO_LOOP: // Do while loops, condition at end
            gen_asm_do_loop(node, ctx);
            break;
        case AST_BREAK:
            asm_add(2, "jmp ", ctx.last_end_label);
            gen_asm(node->next, ctx);
            break;
        case AST_CONTINUE:
            // This doesn't work for for loops, we need to execute the increment too
            asm_add(2, "jmp ", ctx.last_start_label);
            gen_asm(node->next, ctx);
            break;
        case AST_SWITCH:
            gen_asm_switch(node, ctx);
            break;
        case AST_CASE:
            gen_asm_case(node, ctx);
            break;
        case AST_RETURN:
            gen_asm_return(node, ctx);
            break;
        case AST_LABEL:
            asm_add(4, ".L", node->literal, ":", " ; Goto label");
            gen_asm(node->next, ctx);
            break;
        case AST_GOTO:
            asm_add(4, "jmp ", ".L", node->literal, " ; Goto");
            gen_asm(node->next, ctx);
            break;
        case AST_END:
            break;
        case AST_STMT:
        case AST_NULL_STMT:
            gen_asm(node->next, ctx);
            break;
        default:
            codegen_error("Encountered AST Node which has no codegen capability yet!");
            return;
    }
}

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node, AsmContext ctx) {
    if (node->expr_type == EXPR_LITERAL) {
        asm_add(2, "mov rax, ", node->literal);
    }
    else if (node->expr_type == EXPR_VAR) {
        char* sp2 = var_to_stack_ptr(&node->var);
        // Handle various variable type sizes
        if (node->var.type.bytes == 1 || node->var.type.bytes == 2) {
            asm_add(2, "movzx rax, ", sp2); // Zeroes the upper unused bits
        }
        else if (node->var.type.bytes == 4) {
            asm_add(2, "mov eax, ", sp2); // Moving into eax automatically zeroes upper bits
        }
        else if (node->var.type.bytes == 8) {
            asm_add(2, "mov rax, ", sp2);
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
        gen_asm_unary_op_int(node, ctx);
        if (node->top_level_expr) {
            gen_asm(node->next, ctx);
        }
    }
    else if (node->expr_type == EXPR_BINOP) {
        gen_asm_binary_op_int(node, ctx);
        if (node->top_level_expr) {
            gen_asm(node->next, ctx);
        }
    }
    else {
        codegen_error("Non-supported expression type encountered!");
    }
}

void gen_asm_const_expr(ASTNode* node, AsmContext ctx) {
    // Assignment when a variable is global inserts into the data section
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
            sprintf(buf, "%d", node->cast_type.bytes);
            asm_add(2, "mov rax, ", buf);
            break;
        default:
            codegen_error("Unsupported unary operation found!");
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
            codegen_error("Unsupported binary operation found!");
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

// Generate assembly for a function call
void gen_asm_func_call(ASTNode* node, AsmContext ctx) {
    /* 
    x86 Linux Calling convention
    Integer arguments: RDI, RSI, RDX, RCX, R8, R9
    Floating point argument: XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6 and XMM7
    Then rest on stack
    Callee-saved RBX, RSP, RBP, and R12–R15
    Return: RAX 
    */
    static char *reg_strs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    asm_add_com("; Expression function call");
    // Evaluate arguments
    ASTNode* current_arg = node->args;
    for (int i = 0; i <  node->func.param_count; i++) { // Current arg is somehow corrupted when the loop starts
        gen_asm(current_arg, ctx);
        asm_add(3, "mov ", reg_strs[i], ", rax");
        asm_add(2, "push ", reg_strs[i]);
        current_arg = current_arg->next;
    }
    // Make everything ready for call
    for (size_t i = 0; i < node->func.param_count; i++){
        asm_add(2, "pop ", reg_strs[i]);
    }
    asm_add(2, "call ", node->func.name);
}

void gen_asm_func(ASTNode* node, AsmContext ctx) {
    /* 
    x86 Linux Calling convention
    Integer arguments: RDI, RSI, RDX, RCX, R8, R9
    Floating point argument: XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6 and XMM7
    Then rest on stack
    Callee-saved RBX, RSP, RBP, and R12–R15
    Return: RAX 
    */
    static RegisterEnum arg_regs[6] = {RDI, RSI, RDX, RCX, R8, R9};
    Variable* param = node->func.params;
    ctx.func_return_label = get_next_label_str();
    asm_set_indent(0);
    asm_add_newline();
    asm_add(2, node->func.name, ":");
    asm_set_indent(1);
    asm_add_com("; Setting up function stack pointer");
    asm_add(1, "push rbp");
    asm_add(1, "mov rbp, rsp");
    char stack_space_str[63];
    sprintf(stack_space_str, "%d", func_get_aligned_stack_usage(node->func)); 
    asm_add(3, "sub rsp, ", stack_space_str, " ; Allocate the stack space used by the function");
    // Evaluate arguments
    asm_add_com("; Store passed function arguments");
    for (size_t i = 0; i < node->func.param_count; i++) {
        char* param_ptr = var_to_stack_ptr(param);
        char* reg_str = get_reg_width_str(param->type.bytes, arg_regs[i]);
        asm_add(4, "mov ", param_ptr, ", ", reg_str);
        param++;
        free(param_ptr);
    }
    asm_add_com("; Function code start");
    // Do I need to allocate more stack space here?
    gen_asm(node->body, ctx);
    asm_add_newline();
    // Add return
    asm_add(1, "mov rax, 0 ; Default function return is 0");
    asm_add(2, ctx.func_return_label, ": ; Function return label");
    asm_add(3, "add rsp, ", stack_space_str, " ; Restore stack allocation");
    asm_add(1, "pop rbp");
    asm_add(1, "ret");
    free(ctx.func_return_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for an if conditional node
void gen_asm_if(ASTNode* node, AsmContext ctx) {
    // Calculate conditional
    char* after_label;
    char* else_label;
    asm_add_newline();
    asm_add_com("; Calculating if statement conditional");
    gen_asm(node->cond, ctx); // Value now in RAX
    asm_add(1, "cmp rax, 0");
    if (node->els != NULL) { // There is an else statement
        else_label = get_next_label_str();
        asm_add(3, "je ", else_label, " ; Conditional false => Jump to Else");
        gen_asm(node->body, ctx); // If body
        after_label = get_next_label_str();
        asm_add(3, "jmp ", after_label, " ; Jump to end of if/else after if"); 
        asm_add_com("; Label: Else statement");
        asm_add(3, else_label, ":", " ; Else statement");
        gen_asm(node->els, ctx); // Else body
        asm_add_newline();
        free(else_label);
    }
    else { // No else statement
        after_label = get_next_label_str();
        asm_add(2, "je ", after_label, "; Conditional false => Jump to end of if block");
        gen_asm(node->body, ctx); // If body
        asm_add_newline();
    }
    // Jump label after if
    asm_add(2, after_label, ":", " ;  End of if/else");
    free(after_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for a loop node, condition at start, ex while and for loops
void gen_asm_loop(ASTNode* node, AsmContext ctx) {
    char* loop_start_label = get_next_label_str();
    char* loop_end_label = get_next_label_str();
    // For loop
    // Setup ctx for break/continues
    ctx.last_start_label = loop_start_label;
    ctx.last_end_label = loop_end_label;
    if (node->incr != NULL) { // For loop, jump needs to be near incr
        ctx.last_start_label = get_next_label_str();
    }
    // Add asm
    asm_add_newline();
    asm_add(2, loop_start_label, ":");
    asm_add_com("; Calculating loop statement conditional");
    gen_asm(node->cond, ctx); // Value now in RAX
    asm_add(1, "cmp rax, 0");
    asm_add(2, "je ", loop_end_label, " ; Jump to after loop if conditional is false");
    asm_add_com("; Else, evaluate loop body");
    gen_asm(node->body, ctx);
    if(node->incr != NULL) { // For loop increment
        asm_add(2, ctx.last_start_label, ":", " ; For continue label");
        gen_asm(node->incr, ctx);
        free(ctx.last_start_label);
    }
    asm_add(3, "jmp ",  loop_start_label, " ; Jump to beginning of loop");
    asm_add(3, loop_end_label, ":", " ; End of loop jump label");
    free(loop_start_label);
    free(loop_end_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for a do loop node, condition at end, ex do while loops
void gen_asm_do_loop(ASTNode* node, AsmContext ctx) {
    char* while_start_label = get_next_label_str();
    ctx.last_start_label = while_start_label;
    asm_add_newline();
    asm_add(2, while_start_label, ":");
    asm_add_com("; Evaluate do while body");
    gen_asm(node->body, ctx);
    asm_add_com("; Calculating while statement conditional at end");
    gen_asm(node->cond, ctx); // Value now in RAX
    asm_add(1, "cmp rax, 0");
    asm_add(2, "jne ", while_start_label, " ; Jump to start if conditional is true, otherwise keep going");
    free(while_start_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for a switch statement
void gen_asm_switch(ASTNode* node, AsmContext ctx) {
    AsmContext start_context = ctx;
    asm_add_newline();
    asm_add_com("; Switch statement");

    // Get switch value into rax
    gen_asm(node->cond, ctx);
    // Save it on rbx
    asm_add(1, "mov rbx, rax");

    // Iterate over the linked list of cases
    ValueLabel* case_labels = node->switch_cases;
    ValueLabel* default_label = NULL;
    while (case_labels != NULL) {
        // We need to do a comparison here, and jump if true
        if (case_labels->is_default_case) { // Default case
            default_label = case_labels;
        }
        else { // Normal cases
            char* case_label_str = get_case_label_str(case_labels->id, case_labels->value);
            asm_add(2, "cmp rax, ", case_labels->value);
            asm_add(3, "je ", case_label_str, " ; Jump to the case label if value is equal");
            asm_add(1, "mov rax, rbx"); // Restore rax
            free(case_label_str);
        }
        case_labels = case_labels->next;
    }
    if (default_label != NULL) { // We found a default case
        char* default_label_str = get_case_label_str(default_label->id, "D");
        asm_add(3, "jmp ", default_label_str, " ; Jump to the default case label");
        free(default_label_str);
    }
    char* switch_break_label = get_next_label_str();
    ctx.last_end_label = switch_break_label;
    gen_asm(node->body, ctx);
    // Add label at end for break
    asm_add(2, switch_break_label, ":");
    free(switch_break_label);
    gen_asm(node->next, start_context);
}

// Generate assembly for a switch case
void gen_asm_case(ASTNode* node, AsmContext ctx) {
    char* case_label_str;
    if (node->label.is_default_case) { // Default case
        case_label_str = get_case_label_str(node->label.id, "D");
    }
    else {
        case_label_str = get_case_label_str(node->label.id, node->label.value);
    }
    asm_add(3, case_label_str, ":", " ; Switch case label");
    free(case_label_str);
    gen_asm(node->next, ctx);
}


// Generate assembly for a return statement node
void gen_asm_return(ASTNode* node, AsmContext ctx) {
    asm_add_newline();
    asm_add_com("; Evaluating return expr");
    gen_asm(node->ret, ctx); // Expr is now in RAX
    asm_add(3, "jmp ", ctx.func_return_label, " ; Function return");
    gen_asm(node->next, ctx);
}