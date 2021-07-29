#include "symbol_table.h"


SymbolTable* symbol_table_new() {
    SymbolTable* table = calloc(1, sizeof(SymbolTable));
    table->is_global = true;
    table->label_prefix = 0;
    symbol_table_init(table);
    return table;
}

void symbol_table_init(SymbolTable* table) {
    table->children_ptrs = malloc(sizeof(SymbolTable*));
    table->children_max_count = 1;
    table->vars = calloc(1, sizeof(Variable));
    table->var_max_count = 1;
    table->funcs = calloc(1, sizeof(Function));
    table->func_max_count = 1;
    table->labels = calloc(1, sizeof(ValueLabel));
    table->label_max_count = 1;
    table->objects = calloc(1, sizeof(Object));
    table->object_max_count = 1;
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
    free(table->labels);
    free(table->objects);
    free(table);
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
    child->label_prefix = table->label_prefix; // Case labels
    symbol_table_init(child);
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

// ================ Variables ==================

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
        symbol_error2(var_name, "variable referenced but never declared!");
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

// ================ Functions ==================

Function symbol_table_lookup_func(SymbolTable* table, char* func_name) {
    for (size_t i = 0; i < table->func_count; i++)
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
        symbol_error2(func_name, "function referenced but never declared!");
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
        symbol_table_funcs_realloc(table, table->func_max_count*2);
    }
    table->funcs[table->func_count-1] = func;
    return func;
}

void symbol_table_funcs_realloc(SymbolTable* table, int new_size) {
    table->funcs = realloc(table->funcs, sizeof(Function)*new_size);
    table->func_max_count = new_size;
}

// ================= Labels ====================

// Find all lower scope value labels except if in another switch
// Used for switch cases. Results are in a linked label list
ValueLabel* symbol_table_lookup_switch_case_labels(SymbolTable* table) {
    ValueLabel first_label;
    symbol_table_find_labels_recursively(table, &first_label);
    return first_label.next;
}

void symbol_table_find_labels_recursively(SymbolTable* table, ValueLabel* cur_label) {
    for (size_t i = 0; i < table->label_count; i++)
    {
        ValueLabel* label = &table->labels[i];
        cur_label->next = label;
        cur_label = label;
    }
    cur_label->next = NULL;
    for (size_t i = 0; i < table->children_count; i++)
    {
        SymbolTable* child = table->children_ptrs[i];
        if (!child->is_switch_scope) {
            symbol_table_find_labels_recursively(child, cur_label);
        }
    }
}

int cur_value_label_id = 0;

ValueLabel symbol_table_insert_label(SymbolTable* table, ValueLabel label) {
    table->label_count++;
    if (table->label_count > table->label_max_count) {
        symbol_table_labels_realloc(table, table->label_max_count*2);
    }
    cur_value_label_id++;
    label.id = cur_value_label_id;
    table->labels[table->label_count-1] = label;
    return label;
}

void symbol_table_labels_realloc(SymbolTable* table, int new_size) {
    table->labels = realloc(table->labels, sizeof(ValueLabel)*new_size);
    table->label_max_count = new_size;
}

// ================= Objects ===================

Object symbol_table_lookup_object(SymbolTable* table, char* object_name) {
    // Linear search for now, should be a hashtable later
    for (size_t i = 0; i < table->object_count; i++)
    {
        Object* object = &table->objects[i];
        if (strcmp(object->name, object_name) == 0) {
            // Found it!
            return *object;
        }
    }
    if (table->is_global) {
        // The referenced object is not declared anywhere up
        // to the global scope, error!
        symbol_error2(object_name, "object referenced but never declared!");
    }
    // We did not find the object in this scope, go up a scope
    return symbol_table_lookup_object(table->parent, object_name);
}

Object symbol_table_insert_object(SymbolTable* table, Object object) {
    table->object_count++;
    if (table->object_count > table->object_max_count) {
        symbol_table_objects_realloc(table, table->object_max_count*2);
    }
    table->objects[table->object_count-1] = object;
    return object;
}

void symbol_table_objects_realloc(SymbolTable* table, int new_size) {
    table->objects = realloc(table->objects, sizeof(Object)*new_size);
    table->object_max_count = new_size;
}

// ================= Other =====================
void symbol_error(char* error_message) {
    fprintf(stderr, "Symbol error: %s\n", error_message);
    // We are not manually freeing the memory here, 
    // but as the program is exiting it is fine
    exit(1); 
}

void symbol_error2(char* symbol_name, char* error_message) {
    fprintf(stderr, "Symbol error: \"%s\" %s\n", symbol_name, error_message);
    // We are not manually freeing the memory here, 
    // but as the program is exiting it is fine
    exit(1); 
}