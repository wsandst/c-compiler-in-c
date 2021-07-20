/* 
These functions perform code generation, the third and 
last step of the compiler. It generates x86_64 assembly
from the Abstract Syntax Tree created in the parsing step.
*/
#pragma once
#include "parser.h"
#include "string_helpers.h"

//void write_instr()

// Add assembly
void asm_add(char* str);

void asm_add_return(char* return_val);

char* generate_assembly(AST* ast);

void gen_asm(ASTNode* node);