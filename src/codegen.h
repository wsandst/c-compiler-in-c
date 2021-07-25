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

// Set the indendentation level
void asm_set_indent(int indent);

// Get a C string representing a jump label
char* get_label_str(int label);

// Get the next jump label and increment the global label counter
char* get_next_label_str();

// Generate NASM assembly from the AST
char* generate_assembly(AST* ast);

// Generate assembly from the node. Used recursively
void gen_asm(ASTNode* node);

// Generate assembly for a unary op expression node
void gen_asm_unary_op(ASTNode* node);

// Generate assemly for a binary op expression node
void gen_asm_binary_op(ASTNode* node);

// Throw a codegen error
void codegen_error(char* message);