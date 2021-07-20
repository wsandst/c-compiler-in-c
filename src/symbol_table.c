#include "symbol_table.h"


SymbolTable* symbol_table_new() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->block_stack_offset = 0;
    table->is_global = true;
    table->children_count = 0;
    table->max_children_count = 0;
    return table;
}

// Free the symbol table and its children
void symbol_table_free(SymbolTable* table) {
    for (size_t i = 0; i < table->children_count; i++)
    {
        symbol_table_free(table->children_ptrs[i]);
    }
    if (table->max_children_count != 0) {
        free(table->children_ptrs);
    }
    // Free variables and functions too
    free(table);
}

// Lookup a variable in the symbol table
// If not found in this scope, traverse up the scopes until found
// If the variable does not exist anywhere, throw an error
Variable* symbol_table_lookup_var(SymbolTable* table, char* var_name) {
    return NULL;
}

// Insert a variable in this scope of the symbol table
void symbol_table_insert_var(SymbolTable* table, char* var_name) {

}

// Child vector
SymbolTable* symbol_table_create_child(SymbolTable* table, int stack_offset) {
    table->children_count++;
    if (table->max_children_count == 0) {
        // Allocate if no children are allocated
        table->children_ptrs = malloc(sizeof(SymbolTable*));
        table->max_children_count++;
    } 
    else if (table->children_count > table->max_children_count) { 
        // Reallocate if more space is needed
        symbol_table_children_realloc(table, table->max_children_count*2);
    }
    SymbolTable* child = malloc(sizeof(SymbolTable));
    child->max_children_count = 0;
    child->children_count = 0;
    child->block_stack_offset = stack_offset;
    child->parent = table;
    child->is_global = false;
    table->children_ptrs[table->children_count-1] = child;
    return child;
}

void symbol_table_children_realloc(SymbolTable* table, int new_size) {
    table->children_ptrs = realloc(table->children_ptrs, sizeof(SymbolTable*)*new_size);
    table->max_children_count = new_size;
}

SymbolTable* symbol_table_get_child(SymbolTable* table, int index) {
    return table->children_ptrs[index];
}