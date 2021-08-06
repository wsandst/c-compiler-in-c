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

// ============= ASM writing related ==============
// Add assembly
void asm_add_single(StrVector* asm_src, char* str);

// Add assembly comment
void asm_add_com(char* str);

// Add a newline with proper indentation
void asm_add_newline();

// Add assembly, variable amount of strings which are combined
// A newline and proper indentation is added beforehand
void asm_add(int n, ...);

// Set the indendentation level and update
void asm_set_indent(int indent);

// Update indentation level based on global indent_level
void asm_update_indent();

// Add to the indent level and update
void asm_add_indent(int amount);

// Get a C string representing a jump label
char* get_label_str(int label);

// Get a C string representing a switch case jump label
char* get_case_label_str(int label, char* value);

// Get the next jump label and increment the global label counter
char* get_next_label_str();

// Get the corresponding byte size register, eg 2, RAX = AX
char* get_reg_width_str(int size, RegisterEnum reg);

char* var_to_stack_ptr(Variable* var);

// ============= ASM Generation ================

// Generate NASM assembly from the AST
char* generate_assembly(AST* ast, SymbolTable* symbols);

// Generate assembly from the node. Used recursively
void gen_asm(ASTNode* node, AsmContext ctx);

// Generate globals in the data and bss section
void gen_asm_symbols(SymbolTable* symbols);

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node, AsmContext ctx);

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

void gen_asm_unary_op(ASTNode* node, AsmContext ctx);
void gen_asm_binary_op(ASTNode* node, AsmContext ctx);

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

// Setup short circuiting labels for and and or
void gen_asm_setup_short_circuiting(ASTNode* node, AsmContext* ctx);
// Add short circuiting conditional jump after lhs evaluation for AND/OR
void gen_asm_add_short_circuit_jumps(ASTNode* node, AsmContext ctx);

// Casting between any types
void gen_asm_unary_op_cast(VarType to_type, VarType from_type);