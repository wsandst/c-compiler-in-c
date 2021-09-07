/*
Implementation of code generation functionality related to declarations
Mainly related to global and static variables, as well as
certain initializations 
*/
#include "codegen.h"

void gen_asm_global_symbols(SymbolTable* symbols, AsmContext ctx) {
    // Setup function globals
    // Always add memcpy, used by struct operators
    asm_add_sectionf(&ctx, ctx.asm_data_src, "extern memcpy");

    asm_add_sectionf(&ctx, ctx.asm_data_src, "; External or global functions");
    for (size_t i = 0; i < symbols->func_count; i++) {
        Function func = symbols->funcs[i];
        if (func.is_defined) {
            asm_add_sectionf(&ctx, ctx.asm_data_src, "global %s", func.name);
        }
        else {
            // Undefined functions are set to extern for linker
            asm_add_sectionf(&ctx, ctx.asm_data_src, "extern %s", func.name);
        }
    }
    asm_add_newline(&ctx, ctx.asm_data_src);

    // The variables in this scope are always global
    // .data section, globals with constants
    asm_add_sectionf(&ctx, ctx.asm_data_src, "; Global variables");
    for (size_t i = 0; i < symbols->var_count; i++) {
        Variable var = symbols->vars[i];
        if (!var.type.is_static) {
            gen_asm_global_variable(var, &ctx);
        }
    }
    asm_add_sectionf(&ctx, ctx.asm_data_src, "; Static variables");
    gen_asm_symbols(symbols, ctx);
}

void gen_asm_symbols(SymbolTable* symbols, AsmContext ctx) {
    // Handle static variables
    for (size_t i = 0; i < symbols->var_count; i++) {
        Variable var = symbols->vars[i];
        if (var.type.is_static) {
            gen_asm_static_variable(var, &ctx);
        }
    }

    for (size_t i = 0; i < symbols->children_count; i++) {
        gen_asm_symbols(symbol_table_get_child(symbols, i), ctx);
    }
}

void gen_asm_global_variable(Variable var, AsmContext* ctx) {
    if (var.type.array_has_initializer) { // This is handled by the AST_INIT node instead
        return;
    }
    if (var.type.is_extern) { // No definition for this one
        asm_add_sectionf(ctx, ctx->asm_bss_src, "extern %s", var.name);
        return;
    }
    char* data_width_str = bytes_to_data_width(var.type.bytes);
    char* reserve_width_str = bytes_to_reserve_data_width(var.type.bytes);
    if (var.type.is_array) {
        asm_add_sectionf(ctx, ctx->asm_bss_src, "global %s", var.name);
        asm_add_sectionf(ctx, ctx->asm_bss_src, "G_%s: %s %d", var.name,
                         reserve_width_str, var.type.array_size);
    }
    else if (!var.is_undefined) {
        if (var.const_expr_type == LT_STRING) { // String
            char* label_name = get_next_cstring_label_str(ctx);
            asm_add_sectionf(ctx, ctx->asm_rodata_src, "%s: db `%s`, 0", label_name,
                             var.const_expr);
            asm_add_sectionf(ctx, ctx->asm_data_src, "G_%s: %s %s", var.name,
                             data_width_str, label_name);
        }
        else {
            asm_add_sectionf(ctx, ctx->asm_data_src, "G_%s: %s %s", var.name,
                             data_width_str, var.const_expr);
        }
    }
    else {
        asm_add_sectionf(ctx, ctx->asm_bss_src, "; Uninitialized global variable");
        asm_add_sectionf(ctx, ctx->asm_bss_src, "G_%s: resb %d", var.name, var.type.bytes);
    }
}

void gen_asm_static_variable(Variable var, AsmContext* ctx) {
    if (var.type.array_has_initializer) { // This is handled by the AST_INIT node instead
        return;
    }
    if (var.type.is_array) {
        asm_add_sectionf(ctx, ctx->asm_bss_src, "global %s", var.name);
        asm_add_sectionf(ctx, ctx->asm_bss_src, "%s.%ds: resq %d", var.name,
                         var.unique_id, var.type.array_size);
    }
    else if (!var.is_undefined) {
        if (var.const_expr_type == LT_STRING) { // String
            char* label_name = get_next_cstring_label_str(ctx);
            asm_add_sectionf(ctx, ctx->asm_rodata_src, "%s: db `%s`, 0", label_name,
                             var.const_expr);
            asm_add_sectionf(ctx, ctx->asm_data_src, "%s.%ds: dq %s", var.name,
                             var.unique_id, label_name);
        }
        else {
            asm_add_sectionf(ctx, ctx->asm_data_src, "%s.%ds: dq %s", var.name,
                             var.unique_id, var.const_expr);
        }
    }
    else {
        asm_add_sectionf(ctx, ctx->asm_bss_src, "; Uninitialized static variable");
        asm_add_sectionf(ctx, ctx->asm_bss_src, "%s.%ds: resb %d", var.name,
                         var.unique_id, var.type.bytes);
    }
}

void gen_asm_array_initializer(ASTNode* node, AsmContext ctx) {
    // If this is a global or a static initializer, we can initialize when defining the asm variable
    int prev_indent_level = ctx.indent_level;
    if (node->var.is_global || node->var.type.is_static) {
        asm_set_indent(&ctx, 0);
        asm_add_newline(&ctx, ctx.asm_data_src);
        ASTNode* arg_node = node->args;
        if (node->var.type.is_static) {
            asm_add_wn_sectionf(
                &ctx, ctx.asm_data_src, "%s.%ds: %s ", node->var.name, node->var.unique_id,
                bytes_to_data_width(get_deref_var_type(node->var.type).bytes));
        }
        else {
            asm_add_wn_sectionf(
                &ctx, ctx.asm_data_src, "G_%s: %s ", node->var.name,
                bytes_to_data_width(get_deref_var_type(node->var.type).bytes));
        }
        for (size_t i = 0; i < node->var.type.array_size; i++) {
            if (arg_node->type != AST_END) { // Grab the argument value
                if (arg_node->expr_type == EXPR_LITERAL) {
                    if (arg_node->literal_type == LT_STRING) {
                        char* label_name = get_next_cstring_label_str(&ctx);
                        asm_add_sectionf(&ctx, ctx.asm_rodata_src, "%s: db `%s`, 0",
                                         label_name, arg_node->literal);
                        asm_add_wn_sectionf(&ctx, ctx.asm_data_src, "%s, ", label_name);
                    }
                    else {
                        asm_add_wn_sectionf(&ctx, ctx.asm_data_src, "%s, ",
                                            arg_node->literal);
                    }
                }
                else if (arg_node->expr_type == EXPR_VAR && arg_node->var.is_global) {
                    if (arg_node->var.type.is_static) {
                        asm_add_wn_sectionf(&ctx, ctx.asm_data_src, "%s.%ds, ",
                                            arg_node->var.name, arg_node->var.unique_id);
                    }
                    else if (arg_node->var.type.is_const) {
                        asm_add_wn_sectionf(&ctx, ctx.asm_data_src, "%s, ",
                                            arg_node->var.const_expr);
                    }
                    else {
                        asm_add_wn_sectionf(&ctx, ctx.asm_data_src, "G_%s, ",
                                            arg_node->var.name);
                    }
                }
                arg_node = arg_node->next;
            }
            else {
                asm_add_wn_sectionf(&ctx, ctx.asm_data_src, "0, ");
            }
        }
        asm_add_newline(&ctx, ctx.asm_data_src);
    }
    else { // Otherwise, we have to move the values into the array
        asm_add_com(&ctx, "; Array initializer");
        int stack_ptr = node->var.stack_offset;
        VarType deref_type = get_deref_var_type(node->var.type);
        int end_stack_ptr = node->var.stack_offset -
                            deref_type.bytes * node->var.type.array_size;
        ASTNode* arg_node = node->args;
        char* addr_size = bytes_to_addr_width(deref_type.bytes);
        while (stack_ptr > end_stack_ptr) {
            if (arg_node->type != AST_END) { // Grab the argument value
                if (arg_node->expr_type == EXPR_LITERAL) {
                    if (arg_node->literal_type == LT_STRING) {
                        char* label_name = get_next_cstring_label_str(&ctx);
                        asm_add_sectionf(&ctx, ctx.asm_rodata_src, "%s: db `%s`, 0",
                                         label_name, arg_node->literal);

                        asm_addf(&ctx, "lea rax, [%s]", label_name);
                        asm_addf(&ctx, "mov %s [rbp-%d], rax", addr_size, stack_ptr);
                    }
                    else {
                        asm_addf(&ctx, "mov %s [rbp-%d], %s", addr_size, stack_ptr,
                                 arg_node->literal);
                    }
                }
                else if (arg_node->expr_type == EXPR_VAR && arg_node->var.is_global) {
                    if (arg_node->var.type.is_static) {
                        asm_addf(&ctx, "mov rax, [%s.%ds]", arg_node->var.name,
                                 arg_node->var.unique_id);
                        asm_addf(&ctx, "mov %s [rbp-%d], rax", addr_size, stack_ptr);
                    }
                    else if (arg_node->var.type.is_const) {
                        asm_addf(&ctx, "mov %s [rbp-%d], %s", addr_size, stack_ptr,
                                 arg_node->var.const_expr);
                    }
                    else {
                        asm_addf(&ctx, "mov rax, [G_%s]", arg_node->var.name);
                        asm_addf(&ctx, "mov %s [rbp-%d], rax", addr_size, stack_ptr);
                    }
                }
                else {
                    codegen_error("Invalid initializer element in array!");
                }
                arg_node = arg_node->next;
            }
            else { // Out of arguments, set to 0
                asm_addf(&ctx, "mov %s [rbp-%d], 0", addr_size, stack_ptr);
            }
            stack_ptr -= deref_type.bytes;
        }
    }
    asm_set_indent(&ctx, prev_indent_level);
}