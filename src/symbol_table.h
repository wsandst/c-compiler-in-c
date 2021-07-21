// The symbol table represents the various variables and functions in a program
// Each scope has its own symbol table and is linked to the parent scope. 
// Then every scope has an inherent stack offset, which determines local variables location
// on the stack
// To find a variable, first search the current scope and traverse all the way to the top
// The symbol table is a tree of scopes. Every time a new scope is made, I make a new child
// Global scope -> function scope -> block scope etc

#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum VarTypeEnum VarTypeEnum;
typedef struct Variable Variable;
typedef struct Function Function;
typedef struct SymbolTable SymbolTable;

enum VarTypeEnum {
  TY_VOID,
  TY_BOOL,
  TY_CHAR,
  TY_SHORT,
  TY_INT,
  TY_LONG,
  TY_FLOAT,
  TY_DOUBLE,
  TY_ENUM,
  TY_PTR,
  TY_STRUCT,
  //TY_UNION,
  //TY_ARRAY,
  //TY_FUNC,
};

struct Variable {
    char* name;
    VarTypeEnum type;
    int size;
    int stack_offset; // How do we calculate this? Simple, 
    // it's just the inherent offset of the symbol table + index in symbol table
    // The inherent offset of a symbol table is 0 at the function level, then 
    // whatever the size of the parent block is when encountered. 
};

// Functions are stored outside of the main AST
struct Function {
    char* name;
    VarTypeEnum return_type;
    VarTypeEnum* params;

    // A function needs to know its body, doesn't it?
    // How do I generate the entire program?
    // I want to go over every function and add them as cod
    // The uppermost AST Node is the AST_PROGRAM, which contains children functions
    // We loop over these and add them as code.
    Function* next_mem; // Temporary
};

// This is a tree of tables
struct SymbolTable {
    bool is_global; // Is this the global scope, at the top?
    int cur_stack_offset;
    SymbolTable* parent;

    // Vector of pointers to the children. This might be better as a linked list?
    int children_count;
    int children_max_count;
    SymbolTable** children_ptrs; 

    // Variable vector. This should later be a hashtable
    int var_count;
    int var_max_count;
    Variable* vars;

    // Function vector. At first this will only be in the global scope,
    // but later I should implement local functions
    int func_count;
    int func_max_count;
    Function* funcs;
};

SymbolTable* symbol_table_new();

// Free the symbol table and its children
void symbol_table_free(SymbolTable* table);

// Lookup a variable in the symbol table
// If not found in this scope, traverse up the scopes until found
// If the variable does not exist anywhere, throw an error
Variable symbol_table_lookup_var(SymbolTable* table, char* var_name);

// Insert a variable in this scope of the symbol table
Variable symbol_table_insert_var(SymbolTable* table, Variable var);

void symbol_table_vars_realloc(SymbolTable* table, int new_size);

// Child vector related
SymbolTable* symbol_table_create_child(SymbolTable* table, int stack_offset);

void symbol_table_children_realloc(SymbolTable* table, int new_size);

SymbolTable* symbol_table_get_child(SymbolTable* table, int index);

void symbol_error(char* error_message);