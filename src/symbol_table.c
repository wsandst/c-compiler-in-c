#include "symbol_table.h"


SymbolTable* symbol_table_new() {
    SymbolTable* table = calloc(1, sizeof(SymbolTable));
    table->is_global = true;
    table->children_ptrs = malloc(sizeof(SymbolTable*));
    table->children_max_count = 1;
    table->vars = calloc(1, sizeof(Variable));
    table->var_max_count = 1;
    table->funcs = calloc(1, sizeof(Function));
    table->func_max_count = 1;
    return table;
}

// Free the symbol table and its children
void symbol_table_free(SymbolTable* table) {
    for (size_t i = 0; i < table->children_count; i++)
    {
        symbol_table_free(table->children_ptrs[i]);
    }
    if (table->children_max_count != 0) {
        free(table->children_ptrs);
    }
    free(table->vars);
    free(table->funcs); // We need to free the func params as well
    free(table);
}

// Lookup a variable in the symbol table
// If not found in this scope, traverse up the scopes until found
// If the variable does not exist anywhere, throw an error
Variable symbol_table_lookup_var(SymbolTable* table, char* var_name) {
    // Linear search for now, should be a hashtable later
    for (size_t i = 0; i < table->var_count; i++)
    {
        Variable* var = &table->vars[i];
        if (strcmp(var->name, var_name) == 0) {
            // Found it!
            return *var;
        }
    }
    if (table->is_global) {
        // The referenced variable is not declared anywhere up
        // to the global scope, error!
        symbol_error("Variable referenced but never declared!");
    }
    // We did not find the variable in this scope, go up a scope
    return symbol_table_lookup_var(table->parent, var_name);
}

// Insert a variable in this scope of the symbol table
// I should probably do a lookup first and give errors incase of redeclaration
// but this works for now
Variable symbol_table_insert_var(SymbolTable* table, Variable var) {
    table->var_count++;
    if (table->var_count > table->var_max_count) {
        symbol_table_vars_realloc(table, table->var_max_count*2);
    }
    table->cur_stack_offset += var.size;
    var.stack_offset = table->cur_stack_offset;
    table->vars[table->var_count-1] = var;
    return var;
}

void symbol_table_vars_realloc(SymbolTable* table, int new_size) {
    table->vars = realloc(table->vars, sizeof(Variable)*new_size);
    table->var_max_count = new_size;
}

// Function related
Function symbol_table_lookup_func(SymbolTable* table, char* func_name) {
    for (size_t i = 0; i < table->var_count; i++)
    {
        Function* func = &table->funcs[i];
        if (strcmp(func->name, func_name) == 0) {
            // Found it!
            return *func;
        }
    }
    if (table->is_global) {
        // The referenced function is not declared anywhere up
        // to the global scope, error!
        symbol_error("Function referenced but never declared!");
    }
    // We did not find the variable in this scope, go up a scope
    return symbol_table_lookup_func(table->parent, func_name);
}

// Insert a variable in this scope of the symbol table
Function symbol_table_insert_func(SymbolTable* table, Function func) {
    if (!table->is_global) {
        symbol_error("Only global functions are allowed, encountered local definition");
    }
    table->func_count++;
    if (table->func_count > table->func_max_count) {
        symbol_table_vars_realloc(table, table->func_max_count*2);
    }
    // I need to take account for the param variables on the stack here
    // For now, just add param count * 4 to the stack offset
    // How do local functions work? No clue
    table->funcs[table->func_count-1] = func;
    return func;
}

void symbol_table_funcs_realloc(SymbolTable* table, int new_size) {
    table->funcs = realloc(table->funcs, sizeof(Function)*new_size);
    table->func_max_count = new_size;
}


// Child vector
SymbolTable* symbol_table_create_child(SymbolTable* table, int stack_offset) {
    table->children_count++;
    if (table->children_count > table->children_max_count) { 
        // Reallocate if more space is needed
        symbol_table_children_realloc(table, table->children_max_count*2);
    }
    SymbolTable* child = calloc(1, sizeof(SymbolTable));
    child->cur_stack_offset = stack_offset;
    child->parent = table;
    child->children_ptrs = malloc(sizeof(SymbolTable*));
    child->children_max_count = 1;
    child->vars = calloc(1, sizeof(Variable));
    child->var_max_count = 1;
    child->funcs = calloc(1, sizeof(Function));
    child->func_max_count = 1;
    table->children_ptrs[table->children_count-1] = child;
    return child;
}

void symbol_table_children_realloc(SymbolTable* table, int new_size) {
    table->children_ptrs = realloc(table->children_ptrs, sizeof(SymbolTable*)*new_size);
    table->children_max_count = new_size;
}

SymbolTable* symbol_table_get_child(SymbolTable* table, int index) {
    return table->children_ptrs[index];
}

void symbol_error(char* error_message) {
    fprintf(stderr, "Symbol error: %s\n", error_message);
    // We are not manually freeing the memory here, 
    // but as the program is exiting it is fine
    exit(1); 
}