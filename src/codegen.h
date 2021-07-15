/* 
These functions perform the code generation step of the compiler. 
They generate x86_64 Assembly from the AST 
*/
#pragma once
#include "parser.h"

char* generate_assembly(AST ast);