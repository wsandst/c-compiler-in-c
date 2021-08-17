/* 
These functions perform code generation, the third and 
last step of the compiler. It generates x86_64 assembly
from the Abstract Syntax Tree created in the parsing step.
The code generation is divided up between two files,
codegen_expr.c performs everything related to expressions
and operations, and codegen.c file does everything else
*/
#pragma once
#include <stdarg.h>

#include "parser.h"
#include "util/string_helpers.h"

// Contains various context data required
struct AsmContext {
    // Global state
    StrVector* asm_rodata_src;
    StrVector* asm_data_src;
    StrVector* asm_bss_src;
    StrVector* asm_text_src;
    char** asm_indent_str;
    int* label_count;
    int* cstring_label_count;
    // Used by break and continue
    char* last_start_label; // Latest start label for loops
    char* last_end_label; // Latest end label for loops and switch
    char* func_return_label;
    // Short circuiting
    char* and_short_circuit_label;
    char* or_short_circuit_label;
    bool and_end_node;
    bool or_end_node;
};

enum RegisterEnum {
    RAX, RBX, RCX, RDX, RSI, RDI,
    R8, R9, R10, R11, R12, R13, R14, R15,
};

typedef struct AsmContext AsmContext;
typedef enum RegisterEnum RegisterEnum;

// ============= ASM writing related =============
// Add a str to the assembly src
void asm_add(StrVector* src, char* str);
// Add to a specific assembly src section, like the .data section
void asm_add_section(AsmContext* ctx, StrVector* section, int n, ...);
// Add assembly using a format string, also adds newline and indentation
void asm_addf(AsmContext* ctx, char* format_string, ...);
// Add to a specific assembly src section, like the .data section, formatted
void asm_add_sectionf(AsmContext* ctx, StrVector* section, char* format_string, ...);
// Add to a specific assembly src section, like the .data section, formatted, without a newline
void asm_add_wn_sectionf(AsmContext* ctx, StrVector* section, char* format_string, ...);
// Add assembly comment
void asm_add_com(AsmContext* ctx, char* str);
// Add a newline with proper indentation
void asm_add_newline(AsmContext* ctx, StrVector* asm_src);
// Set the indendentation level and update the indentation string
void asm_set_indent(AsmContext* ctx, int indent);

// Create a new AsmContext object
AsmContext asm_context_new();
// Free memory used by an AsmContext object
void asm_context_free(AsmContext* ctx);
// Join the four assembly sections into a single string
char* asm_context_join_srcs(AsmContext* ctx);

// Get a C string representing a jump label
char* get_label_str(int label);
// Get a C string representing a switch case jump label
char* get_case_label_str(int label, char* value);
// Get the next jump label and increment the global label counter
char* get_next_label_str(AsmContext* ctx);
// Get the next label for constant c-strings, used for string literals
char* get_next_cstring_label_str(AsmContext* ctx);

// Get the corresponding byte size register, eg 2, RAX -> AX
char* get_reg_width_str(VarType var_type, RegisterEnum reg);
// Get the address size corresponding to bytes, ex 8->qword or 4->dword
char* bytes_to_addr_width(int bytes);
// Get the value size corresponding to bytes, ex 1->db, 2->dw, etc...
char* bytes_to_data_width(int bytes);
// Get stack adress of a variable, ex qword [rbp - 8]
char* var_to_stack_ptr(Variable* var);
// Get the corresponding move instr for a certain memory size, ex movzx for 2
char* get_move_instr_for_var_type(VarType var_type);
// Get deref type of variable type
VarType get_deref_var_type(VarType var_type);

// ============= ASM Generation ================

// Generate NASM assembly from the AST
char* generate_assembly(AST* ast, SymbolTable* symbols);

// Generate assembly from the node. Used recursively
void gen_asm(ASTNode* node, AsmContext ctx);

// Generate globals in the data and bss section
void gen_asm_global_symbols(SymbolTable* symbols, AsmContext ctx);

// Generate assembly for certain symbols (static etc)
void gen_asm_symbols(SymbolTable* symbols, AsmContext ctx);

// Generate assembly for a global variable declaration
void gen_asm_global_variable(Variable var, AsmContext* ctx);

// Generate assembly for a static variable declaration
void gen_asm_static_variable(Variable var, AsmContext* ctx);

// Generate assembly for an array initializer
void gen_asm_array_initializer(ASTNode* node, AsmContext ctx);

// Generate assembly for a function definition
void gen_asm_func(ASTNode* node, AsmContext ctx);

// Generate assembly for a function call
void gen_asm_func_call(ASTNode* node, AsmContext ctx);

// Generate assembly for an if conditional node
void gen_asm_if(ASTNode* node, AsmContext ctx);

// Generate assembly for a loop node, condition at start, ex while and for loops
void gen_asm_loop(ASTNode* nodel, AsmContext ctx);

// Generate assembly for a do loop node, condition at end, ex do while loops
void gen_asm_do_loop(ASTNode* node, AsmContext ctx);

// Generate assembly for a switch statement
void gen_asm_switch(ASTNode* node, AsmContext ctx);

// Generate assembly for a switch case
void gen_asm_case(ASTNode* node, AsmContext ctx);

// Generate assembly for a return statement node
void gen_asm_return(ASTNode* node, AsmContext ctx);

// Generate assembly for a global variable declaration
void gen_asm_global_dec(ASTNode* node, AsmContext ctx);

// Throw a codegen error
void codegen_error(char* message);

// =============== Codegen expressions ===================

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node, AsmContext ctx);
// Generate assembly for a literal (int, float, string, char)
void gen_asm_literal(ASTNode* node, AsmContext ctx);
// Generate assembly for a unary op on any type
void gen_asm_unary_op(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op on any types
void gen_asm_binary_op(ASTNode* node, AsmContext ctx);
// Generate assembly for the unary op '&' on any type
void gen_asm_unary_op_address(ASTNode* node, AsmContext ctx);

// =============== Integer operations ===============
// Generate assembly for an integer unary op expression node
void gen_asm_unary_op_int(ASTNode* node, AsmContext ctx);
// Generate assembly for an integer binary op expression node
void gen_asm_binary_op_int(ASTNode* node, AsmContext ctx);
// Generate assembly for an integer binary op assignment expression node
void gen_asm_binary_op_assign_int(ASTNode* node, AsmContext ctx);
// Generate assembly for an integer binary op AND node (with short circuiting)
void gen_asm_binary_op_and_int(ASTNode* node, AsmContext ctx);
// Generate assembly for an integer binary op OR node (with short circuiting)
void gen_asm_binary_op_or_int(ASTNode* node, AsmContext ctx);

// =============== Float operations ===============
// Generate assembly for a float unary op expression node
void gen_asm_unary_op_float(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op expression node
void gen_asm_binary_op_float(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op assignment expression node
void gen_asm_binary_op_assign_float(ASTNode* node, AsmContext ctx);

// =============== Pointer operations ===============
// Generate assembly for a float unary op expression node
void gen_asm_unary_op_ptr(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op expression node
void gen_asm_binary_op_ptr(ASTNode* node, AsmContext ctx);
// Generate assembly for pointer dereferencing operator
void gen_asm_unary_op_ptr_deref(ASTNode* node, AsmContext ctx);
// Multiply int RBX value with size of pointer, used for adding and subtracting
void gen_asm_binary_op_load_ptr_size(ASTNode* node, AsmContext ctx);

// Setup short circuiting labels for and and or
void gen_asm_setup_short_circuiting(ASTNode* node, AsmContext* ctx);
// Add short circuiting conditional jump after lhs evaluation for AND/OR
void gen_asm_add_short_circuit_jumps(ASTNode* node, AsmContext ctx);

// Casting between any types
void gen_asm_unary_op_cast(AsmContext ctx, VarType to_type, VarType from_type);