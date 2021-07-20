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

char* generate_assembly(AST *ast);