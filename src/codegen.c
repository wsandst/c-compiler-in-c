#include "codegen.h"

// Toggle for including comments in the asm output code

const bool INCLUDE_COMMENTS = true;

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

void asm_add(StrVector* src, char* str) {
    str_vec_push(src, str);
}

void asm_addf(AsmContext* ctx, char* format_string, ...) {
    va_list vl;
    va_start(vl, format_string);
    static char buf[256];
    vsnprintf(buf, 255, format_string, vl);
    va_end(vl);
    asm_add_newline(ctx, ctx->asm_text_src);
    asm_add(ctx->asm_text_src, buf);
}

void asm_add_sectionf(AsmContext* ctx, StrVector* section, char* format_string, ...) {
    va_list vl;
    va_start(vl, format_string);
    static char buf[256];
    vsnprintf(buf, 255, format_string, vl);
    va_end(vl);
    asm_add_newline(ctx, section);
    asm_add(section, buf);
}

void asm_add_wn_sectionf(AsmContext* ctx, StrVector* section, char* format_string, ...) {
    va_list vl;
    va_start(vl, format_string);
    static char buf[256];
    vsnprintf(buf, 255, format_string, vl);
    va_end(vl);
    asm_add(section, buf);
}

void asm_add_com(AsmContext* ctx, char* comment) {
    if (INCLUDE_COMMENTS) {
        asm_add_newline(ctx, ctx->asm_text_src);
        asm_add(ctx->asm_text_src, comment);
    }
}

void asm_add_newline(AsmContext* ctx, StrVector* asm_src) {
    char buf[64];
    snprintf(buf, 63, "\n%s", *ctx->asm_indent_str);
    asm_add(asm_src, buf);
}

void asm_set_indent(AsmContext* ctx, int indent) {
    free(*(ctx->asm_indent_str));
    *ctx->asm_indent_str = str_multiply("    ", indent);
    return;
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

// Get the address size corresponding to bytes, ex 8->qword or 4->dword
char* bytes_to_addr_width(int bytes) {
    switch (bytes) {
        case 8:
            return "qword";
        case 4:
            return "dword";
        case 2:
            return "word";
        case 1:
            return "byte";
        default:
            return "error";
    }
}

// Get the value size corresponding to bytes, ex 1->db, 2->dw, etc...
char* bytes_to_data_width(int bytes) {
    switch (bytes) {
        case 8:
            return "dq";
        case 4:
            return "dd";
        case 2:
            return "dw";
        case 1:
            return "db";
        default:
            return "error";
    }
}

char* var_to_stack_ptr(Variable* var) {
    if (var->type.is_static) {
        static char buf[64];
        snprintf(buf, 63, "[%s.%ds]", var->name, var->unique_id);
        return str_copy(buf);
    }
    else if (var->is_global) {
        static char buf[64];
        snprintf(buf, 63, "[%s]", var->name);
        return str_copy(buf);
    }
    else {
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
                //codegen_error("Unsupported stack byte length encountered (var_to_stack_ptr)");
        }
        
    }
    return NULL;
}

char* get_reg_width_str(VarType var_type, RegisterEnum reg) {
    int index;
    switch (var_type.bytes) {
        case 8:
            index = 3;
            break;
        default:
            index = var_type.bytes / 2;
    }
    return register_enum_to_modifier_strs[reg][index];
}

char* get_move_instr_for_var_type(VarType var_type) {
    if (var_type.bytes == 8) {
        return str_copy("mov rax");
    }
    else if (var_type.bytes == 4) {
        return str_copy("mov eax"); // Moving into eax automatically zeroes upper bits
    }
    else if (var_type.bytes == 1 || var_type.bytes == 2) {
        return str_copy("movzx rax"); // Zeroes the upper unused bits
    }
    return NULL;
}

VarType get_deref_var_type(VarType var_type) {
    var_type.ptr_level--;
    if (var_type.ptr_level == 0) {
        var_type.bytes = var_type.ptr_value_bytes;
    }
    return var_type;
}

char* get_label_str(int label) {
    static char result[64];
    snprintf(result, 63, ".L%d", label);
    return result;
}

char* get_case_label_str(int label, char* value) {
    static char result[64];
    snprintf(result, 63, ".LC%d_%s", label, value);
    return result;
}

char* get_next_label_str(AsmContext* ctx) {
    (*ctx->label_count)++;
    return get_label_str(*ctx->label_count);
}

char* get_next_cstring_label_str(AsmContext* ctx) {
    (*ctx->cstring_label_count)++;
    static char result[64];
    snprintf(result, 63, "STR%d", *ctx->cstring_label_count);
    return result;
}

AsmContext asm_context_new() {
    AsmContext ctx;
    ctx.last_start_label = NULL;
    ctx.last_end_label = NULL;
    ctx.and_short_circuit_label = NULL;
    ctx.or_short_circuit_label = NULL;
    ctx.and_end_node = false;
    ctx.or_end_node = false;
    char* indent_str = calloc(1, sizeof(char));
    char** indent_str_ptr = calloc(1, sizeof(char*));
    *indent_str_ptr = indent_str;
    ctx.asm_indent_str = indent_str_ptr;
    ctx.asm_rodata_src = str_vec_new_ptr(16);
    ctx.asm_data_src = str_vec_new_ptr(16);
    ctx.asm_bss_src = str_vec_new_ptr(16);
    ctx.asm_text_src = str_vec_new_ptr(16);
    str_vec_push(ctx.asm_rodata_src, "\nsection .rodata\n");
    str_vec_push(ctx.asm_data_src, "\nsection .data\n");
    str_vec_push(ctx.asm_bss_src, "\nsection .bss\n");
    str_vec_push(ctx.asm_text_src, "\nsection .text\n");
    ctx.label_count = calloc(1, sizeof(int));
    ctx.cstring_label_count = calloc(1, sizeof(int));
    return ctx;
}

void asm_context_free(AsmContext* ctx) {
    str_vec_free(ctx->asm_rodata_src);
    str_vec_free(ctx->asm_data_src);
    str_vec_free(ctx->asm_bss_src);
    str_vec_free(ctx->asm_text_src);
    free(ctx->asm_rodata_src);
    free(ctx->asm_data_src);
    free(ctx->asm_bss_src);
    free(ctx->asm_text_src);
    free(*ctx->asm_indent_str);
    free(ctx->asm_indent_str);
    free(ctx->label_count);
    free(ctx->cstring_label_count);
}

char* asm_context_join_srcs(AsmContext* ctx) {
    char* asm_rodata_str = str_vec_join(ctx->asm_rodata_src);
    char* asm_data_str = str_vec_join(ctx->asm_data_src);
    char* asm_bss_str = str_vec_join(ctx->asm_bss_src);
    char* asm_text_str = str_vec_join(ctx->asm_text_src);
    // Join the different sections
    char* temp_str1 = str_add(asm_rodata_str, asm_data_str);
    char* temp_str2 = str_add(temp_str1, asm_bss_str);
    char* asm_src_str = str_add(temp_str2, asm_text_str);
    free(temp_str1);
    free(temp_str2);
    free(asm_rodata_str);
    free(asm_data_str);
    free(asm_bss_str);
    free(asm_text_str);
    return asm_src_str;
}

char* generate_assembly(AST* ast, SymbolTable* symbols) {
    // Setup context object
    AsmContext ctx = asm_context_new();

    // Setup globals/functions
    asm_set_indent(&ctx, 0);
    gen_asm_global_symbols(symbols, ctx);

    gen_asm(ast->program, ctx);

    asm_add_newline(&ctx, ctx.asm_data_src);

    // Join the different sections
    char* asm_src_str = asm_context_join_srcs(&ctx);
    
    asm_context_free(&ctx);

    return asm_src_str;
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
            asm_addf(&ctx, "jmp %s", ctx.last_end_label);
            gen_asm(node->next, ctx);
            break;
        case AST_CONTINUE:
            // This doesn't work for for loops, we need to execute the increment too
            asm_addf(&ctx, "jmp %s", ctx.last_start_label);
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
            asm_addf(&ctx, ".L%s: ; Goto label", node->literal);
            gen_asm(node->next, ctx);
            break;
        case AST_GOTO:
            asm_addf(&ctx, "jmp .L%s ; Goto", node->literal);
            gen_asm(node->next, ctx);
            break;
        case AST_INIT:
            gen_asm_array_initializer(node, ctx);
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
    static char *float_reg_strs[8] = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};
    asm_add_com(&ctx, "; Expression function call");

    // Count general and floating point parameters
    Variable* current_func_def_arg = node->func.params;
    ASTNode* current_arg = node->args;
    int float_arg_count = 0;
    int int_arg_count = 0;
    for (int i = 0; i < node->func.call_param_count; i++) {
        VarType arg_type = current_func_def_arg->type;
        if (i > node->func.def_param_count) { // Variadic, use argument type
            arg_type = current_arg->cast_type;
        } 
        if (arg_type.type == TY_FLOAT && arg_type.ptr_level == 0) {
            float_arg_count++;
        }
        else {
            int_arg_count++;
        }
        current_arg = current_arg->next;
        if (i < (node->func.def_param_count-1)) {
            current_func_def_arg++;
        }
    }

    // Add non-register arguments to the stack
    current_arg = node->args_end->prev;
    current_func_def_arg = node->func.params + node->func.def_param_count - 1;
    int temp_float_param_count = float_arg_count;
    int temp_int_param_count = int_arg_count;
    for (int i = node->func.call_param_count; i > 0; i--) {
        VarType arg_type = current_func_def_arg->type;
        if (i > node->func.def_param_count) {
            // Variadic argument, we don't want to cast
            arg_type = current_arg->cast_type;
        } 
        if (arg_type.type == TY_FLOAT && arg_type.ptr_level == 0) {
            if (temp_float_param_count > 8) {
                gen_asm(current_arg, ctx);
                gen_asm_unary_op_cast(ctx, arg_type, current_arg->cast_type);
                asm_addf(&ctx, "movq rax, xmm0");
                asm_addf(&ctx, "push rax");
            }
            temp_float_param_count--;
        }
        else {
            if (temp_int_param_count > 6) {
                // Cast function parameter if necessary
                gen_asm(current_arg, ctx);
                gen_asm_unary_op_cast(ctx, arg_type, current_arg->cast_type);
                asm_addf(&ctx, "push rax");
            }
            temp_int_param_count--;
        }
        if (current_func_def_arg > node->func.params) {
            current_func_def_arg--;
        }
        current_arg = current_arg->prev;
    }

    // Push up to 8 floating point args, used for passing to regs
    current_arg = node->args_end->prev;
    current_func_def_arg = node->func.params + node->func.def_param_count - 1;
    temp_float_param_count = float_arg_count;
    for (int i = node->func.call_param_count; i > 0; i--) {
        VarType arg_type = current_func_def_arg->type;
        if (i > node->func.def_param_count) {
            // Variadic argument, we don't want to cast
            arg_type = current_arg->cast_type;
        } 
        if (arg_type.type == TY_FLOAT && arg_type.ptr_level == 0) {
            if (temp_float_param_count <= 8) {
                gen_asm(current_arg, ctx);
                gen_asm_unary_op_cast(ctx, arg_type, current_arg->cast_type);
                asm_addf(&ctx, "movq rax, xmm0");
                asm_addf(&ctx, "push rax");
            }
            temp_float_param_count--;
        }
        if (current_func_def_arg > node->func.params) {
            current_func_def_arg--;
        }
        current_arg = current_arg->prev;
    }

    // Push up to 6 int args, used for passing to regs
    current_arg = node->args_end->prev;
    current_func_def_arg = node->func.params + node->func.def_param_count - 1;
    temp_int_param_count = int_arg_count;
    for (int i = node->func.call_param_count; i > 0; i--) {
        VarType arg_type = current_func_def_arg->type;
        if (i > node->func.def_param_count) {
            // Variadic argument, we don't want to cast
            arg_type = current_arg->cast_type;
        } 
        if (!(arg_type.type == TY_FLOAT && arg_type.ptr_level == 0)) {
            if (temp_int_param_count <= 6) {
                gen_asm(current_arg, ctx);
                gen_asm_unary_op_cast(ctx, arg_type, current_arg->cast_type);
                asm_addf(&ctx, "push rax");
            }
            temp_int_param_count--;
        }
        if (current_func_def_arg > node->func.params) {
            current_func_def_arg--;
        }
        current_arg = current_arg->prev;
    }

    /* 
    Stack now looks like this:
    | TOP RSP
    | ------
    | INT REGS (up to first 6)
    | FLOAT REGS (up to first 8)
    | REST OF ARGS
    | ...
    -----
    */

    // Pop INT REGS parameters into their respective function regs
    int min_int_pop = min(int_arg_count, 6);
    for (int i = 0; i < min_int_pop; i++) {
        asm_addf(&ctx, "pop rax");
        asm_addf(&ctx, "mov %s, rax", reg_strs[i]);
    }
    // Pop FLOAT REGS parameters into their respective function regs
    int min_float_pop = min(float_arg_count, 8);
    for (int i = 0; i < min_float_pop; i++) {
        asm_addf(&ctx, "pop rax");
        asm_addf(&ctx, "movq %s, rax", float_reg_strs[i]);
    }
    if (node->func.is_variadic) { 
        // Pass floating point reg count in AL in variadic functions
        asm_addf(&ctx, "mov al, %d", min_float_pop);
    }

    asm_addf(&ctx, "call %s", node->func.name);

    // Restore the stack space used by REST OF ARGS
    int pop_count = max(int_arg_count - 6, 0) + max(float_arg_count - 8, 0);
    asm_addf(&ctx, "add rsp, %d", pop_count*8);
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
    static char *float_reg_strs[8] = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};
    Variable* param = node->func.params;
    ctx.func_return_label = str_copy(get_next_label_str(&ctx));
    asm_set_indent(&ctx, 0);
    asm_add_newline(&ctx, ctx.asm_text_src);
    asm_addf(&ctx, "%s:", node->func.name);
    asm_set_indent(&ctx, 1);
    asm_add_com(&ctx, "; Setting up function stack pointer");
    asm_addf(&ctx, "push rbp");
    asm_addf(&ctx, "mov rbp, rsp");
    int stack_space = func_get_aligned_stack_usage(node->func);
    asm_addf(&ctx, "sub rsp, %d ; Allocate the stack space used by the function", stack_space);
    // Evaluate arguments
    asm_add_com(&ctx, "; Store passed function arguments");
    int int_arg_count = 0;
    int float_arg_count = 0;
    int stack_arg_count = 0;
    for (int i = 0; i < node->func.def_param_count; i++) {
        char* param_ptr = var_to_stack_ptr(param);
        if (param->type.type == TY_INT || param->type.ptr_level > 0) {
            if (int_arg_count < 6) { // Pass by register
                char* reg_str = get_reg_width_str(param->type, arg_regs[int_arg_count]);
                asm_addf(&ctx, "mov %s, %s", param_ptr, reg_str);
            }
            else { // Pass by stack
                asm_addf(&ctx, "mov rax, qword [rbp+%d]", 8*(stack_arg_count+2));
                char* reg_str = get_reg_width_str(param->type, RAX);
                asm_addf(&ctx, "mov %s, %s", param_ptr, reg_str);
                stack_arg_count++;
            }
            int_arg_count++;
        }
        else if (param->type.type == TY_FLOAT) {
            if (float_arg_count < 8) {
                asm_addf(&ctx, "movq %s, %s", param_ptr, float_reg_strs[float_arg_count]);
            }
            else {
                asm_addf(&ctx, "mov rax, qword [rbp+%d]", 8*(stack_arg_count+2));
                char* reg_str = get_reg_width_str(param->type, RAX);
                asm_addf(&ctx, "mov %s, %s", param_ptr, reg_str);
                stack_arg_count++;
            }
            float_arg_count++;
        }
        else {
            codegen_error("Unsupported function argument type in function definition");
        }
        free(param_ptr);
        param++;
    }
    asm_add_com(&ctx, "; Function code start");
    // Do I need to allocate more stack space here?
    gen_asm(node->body, ctx);
    asm_add_newline(&ctx, ctx.asm_text_src);
    // Add return
    asm_addf(&ctx, "mov rax, 0 ; Default function return is 0");
    asm_addf(&ctx, "%s: ; Function return label", ctx.func_return_label);
    asm_addf(&ctx, "add rsp, %d ; Restore function stack allocation", stack_space);
    asm_addf(&ctx, "pop rbp");
    asm_addf(&ctx, "ret");
    free(ctx.func_return_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for an if conditional node
void gen_asm_if(ASTNode* node, AsmContext ctx) {
    // Calculate conditional
    char* after_label;
    char* else_label;
    asm_add_newline(&ctx, ctx.asm_text_src);
    asm_add_com(&ctx, "; Calculating if statement conditional");
    gen_asm(node->cond, ctx); // Value now in RAX
    asm_addf(&ctx, "cmp rax, 0");
    if (node->els != NULL) { // There is an else statement
        else_label = str_copy(get_next_label_str(&ctx));
        asm_addf(&ctx, "je %s, ; Conditional false -> Jump to Else", else_label);
        gen_asm(node->body, ctx); // If body
        after_label = str_copy(get_next_label_str(&ctx));
        asm_addf(&ctx, "jmp %s ; Jump to end of if/else after if", after_label); 
        asm_add_com(&ctx, "; Label: Else statement");
        asm_addf(&ctx, "%s: ; Else statement", else_label);
        gen_asm(node->els, ctx); // Else body
        asm_add_newline(&ctx, ctx.asm_text_src);
        free(else_label);
    }
    else { // No else statement
        after_label = str_copy(get_next_label_str(&ctx));
        asm_addf(&ctx, "je %s ; Conditional false => Jump to end of if block", after_label);
        gen_asm(node->body, ctx); // If body
        asm_add_newline(&ctx, ctx.asm_text_src);
    }
    // Jump label after if
    asm_addf(&ctx, "%s: ;  End of if/else", after_label);
    free(after_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for a loop node, condition at start, ex while and for loops
void gen_asm_loop(ASTNode* node, AsmContext ctx) {
    char* loop_start_label = str_copy(get_next_label_str(&ctx));
    char* loop_end_label = str_copy(get_next_label_str(&ctx));
    // For loop
    // Setup ctx for break/continues
    ctx.last_start_label = loop_start_label;
    ctx.last_end_label = loop_end_label;
    if (node->incr != NULL) { // For loop, jump needs to be near incr
        ctx.last_start_label = str_copy(get_next_label_str(&ctx));
    }
    // Add asm
    asm_add_newline(&ctx, ctx.asm_text_src);
    asm_addf(&ctx, "%s:", loop_start_label);
    asm_add_com(&ctx, "; Calculating loop statement conditional");
    gen_asm(node->cond, ctx); // Value now in RAX
    asm_addf(&ctx, "cmp rax, 0");
    asm_addf(&ctx, "je %s ; Jump to after loop if conditional is false", loop_end_label);
    asm_add_com(&ctx, "; Else, evaluate loop body");
    gen_asm(node->body, ctx);
    if(node->incr != NULL) { // For loop increment
        asm_addf(&ctx, "%s: ; For continue label", ctx.last_start_label);
        gen_asm(node->incr, ctx);
        free(ctx.last_start_label);
    }
    asm_addf(&ctx, "jmp %s ; Jump to beginning of loop", loop_start_label);
    asm_addf(&ctx, "%s: ; End of loop jump label", loop_end_label);
    free(loop_start_label);
    free(loop_end_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for a do loop node, condition at end, ex do while loops
void gen_asm_do_loop(ASTNode* node, AsmContext ctx) {
    char* while_start_label = str_copy(get_next_label_str(&ctx));
    ctx.last_start_label = while_start_label;
    asm_add_newline(&ctx, ctx.asm_text_src);
    asm_addf(&ctx, "%s:", while_start_label);
    asm_add_com(&ctx, "; Evaluate do while body");
    gen_asm(node->body, ctx);
    asm_add_com(&ctx, "; Calculating while statement conditional at end");
    gen_asm(node->cond, ctx); // Value now in RAX
    asm_addf(&ctx, "cmp rax, 0");
    asm_addf(&ctx, "jne %s ; Jump to start if conditional is true, otherwise keep going", while_start_label);
    free(while_start_label);
    gen_asm(node->next, ctx);
}

// Generate assembly for a switch statement
void gen_asm_switch(ASTNode* node, AsmContext ctx) {
    AsmContext start_context = ctx;
    asm_add_newline(&ctx, ctx.asm_text_src);
    asm_add_com(&ctx, "; Switch statement");

    // Get switch value into rax
    gen_asm(node->cond, ctx);
    // Save it on rbx
    asm_addf(&ctx, "mov rbx, rax");

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
            asm_addf(&ctx, "cmp rax, %s", case_labels->value);
            asm_addf(&ctx, "je %s ; Jump to the case label if value is equal", case_label_str);
            asm_addf(&ctx, "mov rax, rbx"); // Restore rax
        }
        case_labels = case_labels->next;
    }
    if (default_label != NULL) { // We found a default case
        char* default_label_str = get_case_label_str(default_label->id, "D");
        asm_addf(&ctx, "jmp %s ; Jump to the default case label", default_label_str);
    }
    char* switch_break_label = str_copy(get_next_label_str(&ctx));
    ctx.last_end_label = switch_break_label;
    gen_asm(node->body, ctx);
    // Add label at end for break
    asm_addf(&ctx, "%s:", switch_break_label);
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
    asm_addf(&ctx, "%s: ; Switch case label", case_label_str);
    gen_asm(node->next, ctx);
}

// Generate assembly for a return statement node
void gen_asm_return(ASTNode* node, AsmContext ctx) {
    asm_add_newline(&ctx, ctx.asm_text_src);
    asm_add_com(&ctx, "; Evaluating return expr");
    gen_asm(node->ret, ctx); // Expr is now in RAX
    // Cast to return type
    gen_asm_unary_op_cast(ctx, node->cast_type, node->ret->cast_type);
    asm_addf(&ctx, "jmp %s ; Function return", ctx.func_return_label);
    gen_asm(node->next, ctx);
}