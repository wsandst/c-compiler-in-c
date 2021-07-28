/* 
These functions perform code generation, the third and 
last step of the compiler. It generates x86_64 assembly
from the Abstract Syntax Tree created in the parsing step.
*/
#pragma once
#include <stdarg.h>

#include "parser.h"
#include "string_helpers.h"

//void write_instr()

// Add assembly
void asm_add_single(char* str);

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

// Get the next jump label and increment the global label counter
char* get_next_label_str();

// Generate NASM assembly from the AST
char* generate_assembly(AST* ast);

// Generate assembly from the node. Used recursively
void gen_asm(ASTNode* node);

// Generate assembly for an expression node
void gen_asm_expr(ASTNode* node);

// Generate assembly for a unary op expression node
void gen_asm_unary_op(ASTNode* node);

// Generate assembly for a binary op expression node
void gen_asm_binary_op(ASTNode* node);

// Generate assembly for a function definition
void gen_asm_func(ASTNode* node);

// Generate assembly for an if conditional node
void gen_asm_if(ASTNode* node);

// Generate assembly for a loop node, condition at start, ex while and for loops
void gen_asm_loop(ASTNode* node);

// Generate assembly for a do loop node, condition at end, ex do while loops
void gen_asm_do_loop(ASTNode* node);

// Generate assembly for a return statement node
void gen_asm_return(ASTNode* node);

// Throw a codegen error
void codegen_error(char* message);