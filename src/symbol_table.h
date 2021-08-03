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
#include <math.h>

#include "util/string_helpers.h"

typedef enum VarTypeEnum VarTypeEnum;
typedef struct VarType VarType;
typedef struct Variable Variable;
typedef struct Function Function;
typedef struct ValueLabel ValueLabel;
typedef struct Object Object;
typedef struct SymbolTable SymbolTable;

enum VarTypeEnum {
  TY_VOID,
  TY_BOOL,
  TY_INT,
  TY_FLOAT,
  TY_ENUM,
  TY_STRUCT,
  //TY_UNION,
  //TY_ARRAY,
  //TY_FUNC,
};

struct VarType {
    VarTypeEnum type;
    int bytes;
    bool is_ptr;
    bool is_unsigned;
};

// Variable object
struct Variable {
    char* name;
    VarType type;
    bool is_function_arg;
    int stack_offset;
    bool is_global;
    bool is_undefined;
    char* const_expr;
};

// Function object
struct Function {
    char* name;
    VarType return_type;
    int param_count;
    Variable* params;
    int stack_space_used;
    bool is_defined; // Has this function been defined? Otherwise, declare as extern
};

struct ValueLabel { // Switch case labels
    int id;
    char* value;
    bool is_default_case;
    ValueLabel* next; // Used as linked list for switch
};

enum ObjectTypeEnum {
    STRUCT,
    UNION,
    ENUM,
    TYPEDEF,
};

struct Object { // Structs, unions, enums, typedefs etc
    char* name;
    enum ObjectTypeEnum type;
};

// This is a tree of tables
struct SymbolTable {
    bool is_global; // Is this the global scope, at the top?
    bool is_switch_scope;
    int label_prefix;
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

    // Value Label vector (switch cases)
    int label_count;
    int label_max_count;
    ValueLabel* labels;

    // Object vector (struct, union, enums, typedef)
    int object_count;
    int object_max_count;
    Object* objects;
};

void symbol_error(char* error_message);
void symbol_error2(char* symbol_name, char* error_message);

SymbolTable* symbol_table_new();

void symbol_table_init(SymbolTable* table);

// Free the symbol table and its children
void symbol_table_free(SymbolTable* table);

// ================ Variables ==================
// Lookup a variable in the symbol table
// If not found in this scope, traverse up the scopes until found
// If the variable does not exist anywhere, throw an error
Variable symbol_table_lookup_var(SymbolTable* table, char* var_name);

Variable* symbol_table_lookup_var_ptr(SymbolTable* table, char* var_name);

// Insert a variable in this scope of the symbol table
Variable symbol_table_insert_var(SymbolTable* table, Variable var);

void symbol_table_vars_realloc(SymbolTable* table, int new_size);

Variable* symbol_table_get_function_args(SymbolTable* table);

// Get the max stack space recursively used by the symbol table and all children
int symbol_table_get_max_stack_space(SymbolTable* table);

// ================ Functions ==================
Function symbol_table_lookup_func(SymbolTable* table, char* func_name);

Function* symbol_table_lookup_func_ptr(SymbolTable* table, char* func_name);

// Insert a variable in this scope of the symbol table
Function symbol_table_insert_func(SymbolTable* table, Function func);

void symbol_table_funcs_realloc(SymbolTable* table, int new_size);

// System V ABI requires 16 byte alignment of stack
int func_get_aligned_stack_usage(Function func);

// ================= Labels ====================

// Find all lower scope value labels except if in another switch
// Used for switch cases. Results are in a linked label list
ValueLabel* symbol_table_lookup_switch_case_labels(SymbolTable* table);
// Helper for above
void symbol_table_find_labels_recursively(SymbolTable* table, ValueLabel* cur_label);

ValueLabel symbol_table_insert_label(SymbolTable* table, ValueLabel label);

void symbol_table_labels_realloc(SymbolTable* table, int new_size);

// ================= Objects ===================

Object symbol_table_lookup_object(SymbolTable* table, char* object_name);

Object symbol_table_insert_object(SymbolTable* table, Object object);

void symbol_table_objects_realloc(SymbolTable* table, int new_size);


// =============== Tree related ================
SymbolTable* symbol_table_create_child(SymbolTable* table, int stack_offset);

void symbol_table_children_realloc(SymbolTable* table, int new_size);

SymbolTable* symbol_table_get_child(SymbolTable* table, int index);