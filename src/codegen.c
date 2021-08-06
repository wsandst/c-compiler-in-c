#include "codegen.h"

// Toggle for including comments in the asm output code

const bool INCLUDE_COMMENTS = true;

StrVector asm_text_src;
StrVector asm_data_src;
char* asm_indent_str;
int label_count = 1;
int static_cstring_count = 1;
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
    asm_add_newline(&asm_text_src);
    char* str;
    va_list vl;
    va_start(vl, n);
    for (int i = 0; i < n; i++)
    {
        str = va_arg(vl, char*);
        asm_add_single(&asm_text_src, str);
    }
    va_end(vl);
}

void asm_add_to_data_section(int n, ...) {
    asm_add_newline(&asm_data_src);
    char* str;
    va_list vl;
    va_start(vl, n);
    for (int i = 0; i < n; i++)
    {
        str = va_arg(vl, char*);
        asm_add_single(&asm_data_src, str);
    }
    va_end(vl);
}

void asm_add_com(char* comment) {
    if (INCLUDE_COMMENTS) {
        asm_add_newline(&asm_text_src);
        asm_add_single(&asm_text_src, comment);
    }
}

void asm_add_newline(StrVector* asm_src) {
    char buf[64];
    snprintf(buf, 63, "\n%s", asm_indent_str);
    asm_add_single(asm_src, buf);
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

// Get the address size corresponding to bytes, ex 8->qword or 4->dword
char* bytes_to_addr_size(VarType var_type) {
    switch (var_type.bytes) {
        case 8:
            return str_copy("qword");
        case 4:
            return str_copy("dword");
        case 2:
            return str_copy("word");
        case 1:
            return str_copy("byte");
        default:
            return str_copy("error");
    }
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

char* get_next_cstring_label_str() {
    char result[64];
    sprintf(result, "STR%d", static_cstring_count);
    static_cstring_count += 1;
    return str_copy(result);
}

char* generate_assembly(AST* ast, SymbolTable* symbols) {
    asm_text_src = str_vec_new(16);
    asm_data_src = str_vec_new(16);
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
    asm_add_newline(&asm_text_src);
    asm_add_newline(&asm_data_src);

    char* asm_data_str = str_vec_join(&asm_data_src);
    char* asm_text_str = str_vec_join(&asm_text_src);
    // Join the different sections
    char* asm_src_str = str_add(asm_data_str, asm_text_str);
    str_vec_free(&asm_text_src);
    free(asm_indent_str);
    free(asm_data_str);
    free(asm_text_str);
    return asm_src_str;
}

void gen_asm_symbols(SymbolTable* symbols) {
    // Setup function globals
    asm_add_to_data_section(1, "; External or global functions");
    for (size_t i = 0; i < symbols->func_count; i++) {
        Function func = symbols->funcs[i];
        if (func.is_defined) {
            asm_add_to_data_section(2, "global ", func.name);
        }
        else {
            // Undefined functions are set to extern for linker
            asm_add_to_data_section(2, "extern ", func.name); 
        }
    }   
    asm_add_newline(&asm_data_src);

    // The variables in this scope are always global
    if (symbols->var_count == 0) { // No need if there are no globals
        return;
    }
    // .data section, globals with constants
    asm_add_to_data_section(1, "section .data");
    asm_set_indent(1);
    int undefined_count = 0;
    for (size_t i = 0; i < symbols->var_count; i++) {
        Variable var = symbols->vars[i];
        if (!var.is_undefined) {
            asm_add_to_data_section(2, "global ", var.name);
            asm_add_to_data_section(3, var.name, ": dq ", var.const_expr);
        }
        else {
            undefined_count++;
        }
    }   
    // .bss section, uninitialized globals
    if (undefined_count) { // Only add .bss if necessary
        asm_set_indent(0);
        asm_add_newline(&asm_text_src);
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
    asm_add_newline(&asm_text_src);
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
        if (current_arg->cast_type.ptr_level == 0 && current_arg->cast_type.type == TY_FLOAT) {
            asm_add(1, "movq rax, xmm0"); // We pass f64 here
        }
        asm_add(3, "mov ", reg_strs[i], ", rax");
        asm_add(2, "push ", reg_strs[i]);
        current_arg = current_arg->next;
    }
    // Make everything ready for call
    for (size_t i = node->func.param_count; i > 0; i--){
        asm_add(2, "pop ", reg_strs[i-1]);
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
    asm_add_newline(&asm_text_src);
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
        char* reg_str = get_reg_width_str(param->type, arg_regs[i]);
        asm_add(4, "mov ", param_ptr, ", ", reg_str);
        param++;
        free(param_ptr);
    }
    asm_add_com("; Function code start");
    // Do I need to allocate more stack space here?
    gen_asm(node->body, ctx);
    asm_add_newline(&asm_text_src);
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
    asm_add_newline(&asm_text_src);
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
        asm_add_newline(&asm_text_src);
        free(else_label);
    }
    else { // No else statement
        after_label = get_next_label_str();
        asm_add(2, "je ", after_label, "; Conditional false => Jump to end of if block");
        gen_asm(node->body, ctx); // If body
        asm_add_newline(&asm_text_src);
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
    asm_add_newline(&asm_text_src);
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
    asm_add_newline(&asm_text_src);
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
    asm_add_newline(&asm_text_src);
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
    asm_add_newline(&asm_text_src);
    asm_add_com("; Evaluating return expr");
    gen_asm(node->ret, ctx); // Expr is now in RAX
    // Cast to return type
    gen_asm_unary_op_cast(node->cast_type, node->ret->cast_type);
    asm_add(3, "jmp ", ctx.func_return_label, " ; Function return");
    gen_asm(node->next, ctx);
}