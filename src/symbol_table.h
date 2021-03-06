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
#include <unistd.h>

#include "util/string_helpers.h"

// Represents a literal type
enum LiteralType {
    LT_NONE,
    LT_INT,
    LT_FLOAT,
    LT_STRING,
    LT_CHAR,
};

// Represents the various possible variable types
enum VarTypeEnum {
    TY_VOID,
    TY_BOOL,
    TY_INT,
    TY_FLOAT,
    TY_ENUM,
    TY_STRUCT,
};

// Represents compiler builtin functions
enum BuiltinFuncEnum {
    BUILTIN_NONE,
    BUILTIN_VA_BEGIN,
    BUILTIN_VA_END,
};

typedef enum LiteralType LiteralType;
typedef enum VarTypeEnum VarTypeEnum;
typedef enum ObjectTypeEnum ObjectTypeEnum;
typedef enum BuiltinFuncEnum BuiltinFuncEnum;

typedef struct VarType VarType;

// Represents a variable type (ex int)
struct VarType {
    VarTypeEnum type;
    int bytes;
    int ptr_level; // 0 = not pointer, 1 *, 2 ** etc
    int ptr_value_bytes;
    bool is_unsigned;
    bool is_extern;
    bool is_static;
    bool is_const;
    // Array related information
    bool is_array;
    int array_size;
    bool array_has_initializer;

    // Struct type information
    char* struct_name;
    // Keep track of the members of the struct
    VarType* next_struct_member;
    char* struct_member_name;
    int struct_bytes_offset;
    bool is_struct_member;
    int widest_struct_member;
};

// Type of an Object
enum ObjectTypeEnum {
    OBJ_STRUCT,
    OBJ_ENUM,
    OBJ_TYPEDEF,
};

struct Object { // Structs, unions, enums, typedefs etc
    char* name;
    enum ObjectTypeEnum type;
    VarType typedef_type;
    // Struct related
    VarType struct_type;
    VarType* first_struct_member;
};

typedef struct Object Object;

// Variable object
struct Variable {
    char* name;
    VarType type;
    bool is_function_arg;
    int stack_offset;
    bool is_global;
    bool is_undefined;
    bool is_dereferenced_ptr;
    bool is_enum_member;
    char* const_expr;
    LiteralType const_expr_type;
    int unique_id;
    Object struct_type;
};

typedef struct Variable Variable;

// Function object
struct Function {
    char* name;
    VarType return_type;
    int def_param_count;
    int call_param_count;
    int call_int_param_count;
    int call_float_param_count;
    Variable* params;
    int stack_space_used;
    bool is_defined; // Has this function been defined? Otherwise, declare as extern
    bool is_variadic;
    bool is_builtin;
    BuiltinFuncEnum builtin_type;
};

typedef struct Function Function;

typedef struct ValueLabel ValueLabel;

// Switch case labels
struct ValueLabel {
    int id;
    LiteralType type;
    char* str_value;
    int value;
    bool is_default_case;
    ValueLabel* next; // Used as linked list for switch
    long padding1;
    long padding2;
    long padding3;
};

typedef struct SymbolTable SymbolTable;

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

// Helpers for reporting symbol errors
void symbol_error(char* error_message);
void symbol_error2(char* symbol_name, char* error_message);

// Create a new variable
Variable variable_new();

// Create a dynamically allocated symbol table
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
Variable* symbol_table_insert_var(SymbolTable* table, Variable var);

void symbol_table_vars_realloc(SymbolTable* table, int new_size);

Variable* symbol_table_get_function_args(SymbolTable* table);

// Get the max stack space recursively used by the symbol table and all children
int symbol_table_get_max_stack_space(SymbolTable* table);

// ================ Functions ==================
Function symbol_table_lookup_func(SymbolTable* table, char* func_name);

Function* symbol_table_lookup_func_ptr(SymbolTable* table, char* func_name);

// Insert a variable in this scope of the symbol table
Function* symbol_table_insert_func(SymbolTable* table, Function func);

void symbol_table_funcs_realloc(SymbolTable* table, int new_size);

// System V ABI requires 16 byte alignment of stack
int func_get_aligned_stack_usage(Function func);
// Align a stack address correctly for a type of x bytes
int align_stack_address(int addr, int type_bytes);

int align_stack_address_no_add(int addr, int type_bytes);

// Insert builtin compiler functions
void symbol_table_insert_builtin_funcs(SymbolTable* table);

// ================= Labels ====================

// Find all lower scope value labels except if in another switch
// Used for switch cases. Results are in a linked label list
ValueLabel* symbol_table_lookup_switch_case_labels(SymbolTable* table);
// Helper for above
void symbol_table_find_labels_recursively(SymbolTable* table, ValueLabel* cur_label);

ValueLabel symbol_table_insert_label(SymbolTable* table, ValueLabel label);

void symbol_table_labels_realloc(SymbolTable* table, int new_size);

// ================= Objects ===================

Object* symbol_table_lookup_object(SymbolTable* table, char* object_name,
                                   ObjectTypeEnum type);

Object symbol_table_insert_object(SymbolTable* table, Object object);

void symbol_table_objects_realloc(SymbolTable* table, int new_size);

VarType* symbol_table_struct_lookup_member(Object struct_obj, char* member_name);

// =============== Tree related ================
SymbolTable* symbol_table_create_child(SymbolTable* table, int stack_offset);

void symbol_table_children_realloc(SymbolTable* table, int new_size);

SymbolTable* symbol_table_get_child(SymbolTable* table, int index);