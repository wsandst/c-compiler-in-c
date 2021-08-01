/* 
These functions perform code generation, the third and 
last step of the compiler. It generates x86_64 assembly
from the Abstract Syntax Tree created in the parsing step.
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

typedef struct AsmContext AsmContext;

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

// Generate NASM assembly from the AST
char* generate_assembly(AST* ast, SymbolTable* symbols);

// Generate assembly from the node. Used recursively
void gen_asm(ASTNode* node, AsmContext ctx);

// Generate globals in the data and bss section
void gen_asm_symbols(SymbolTable* symbols);

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node, AsmContext ctx);

// Generate assembly for a constant expression node (used mainly for globals currently)
void gen_asm_const_expr(ASTNode* node, AsmContext ctx);

// Generate assembly for a unary op expression node
void gen_asm_unary_op(ASTNode* node, AsmContext ctx);

// Generate assembly for a binary op expression node
void gen_asm_binary_op(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op assignment expression node
void gen_asm_binary_op_assign(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op AND node (with short circuiting)
void gen_asm_binary_op_and(ASTNode* node, AsmContext ctx);
// Generate assembly for a binary op OR node (with short circuiting)
void gen_asm_binary_op_or(ASTNode* node, AsmContext ctx);
// Setup short circuiting labels for and and or
void gen_asm_setup_short_circuiting(ASTNode* node, AsmContext* ctx);
// Add short circuiting conditional jump after lhs evaluation for AND/OR
void gen_asm_add_short_circuit_jumps(ASTNode* node, AsmContext ctx);

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