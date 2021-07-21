#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "symbol_table.h"

void test_symbol_table();
void test_symbol_table_tree();
void test_symbol_table_vars();

void test_symbol_table() {
    printf("[CTEST] Running symbol table tests...\n");
    test_symbol_table_tree();
    test_symbol_table_vars();
    printf("[CTEST] Passed symbol table tests!\n");
}

// Test child and parent handling of the symbol table tree
void test_symbol_table_tree() {
    SymbolTable* table = symbol_table_new();

    // Test children creation
    SymbolTable* child = symbol_table_create_child(table, 0);
    assert(table->children_count == 1);
    assert(table->children_ptrs[0] == child);
    assert(table == child->parent);
    symbol_table_create_child(table, 0);
    symbol_table_create_child(table, 0);
    assert(table->children_max_count == 4);
    // This should not have invalidated the original child pointer
    assert(child == table->children_ptrs[0]);

    SymbolTable* childchild = symbol_table_create_child(child, 0);
    assert(childchild->parent == child);
    assert(childchild->parent->parent == table);

    symbol_table_free(table);
}

// Test variable insertion and lookup in the symbol table tree
void test_symbol_table_vars() {
    SymbolTable* table = symbol_table_new();
    SymbolTable* child = symbol_table_create_child(table, 0);

    // Inserting
    Variable var;
    var.name = "var1";
    var.size = 1;
    symbol_table_insert_var(table, var);
    assert(table->var_count == 1);
    var.name = "var2";
    var.size = 2;
    symbol_table_insert_var(table, var);
    var.name = "var3";
    var.size = 3;
    symbol_table_insert_var(table, var);
    assert(table->var_count == 3);
    assert(table->var_max_count == 4);

    // Lookup
    assert(symbol_table_lookup_var(table, "var1").size == 1);
    assert(symbol_table_lookup_var(table, "var2").size == 2);
    assert(symbol_table_lookup_var(table, "var3").size == 3);

    var.name = "var4";
    var.size = 4;
    symbol_table_insert_var(child, var);
    assert(symbol_table_lookup_var(child, "var4").size == 4);
    // Check going up a scope
    assert(symbol_table_lookup_var(child, "var1").size == 1);
    // symbol_table_lookup_var(child, "novar"); // This will error!
    symbol_table_free(table);
}