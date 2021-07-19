/* 
These functions perform code generation, the third and 
last step of the compiler. It generates x86_64 assembly
from the Abstract Syntax Tree created in the parsing step.
*/
#pragma once
#include "parser.h"

//void write_instr()

char* generate_assembly(AST ast);