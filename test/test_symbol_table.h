#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "symbol_table.h"

void test_symbol_table();
void test_symbol_table_tree();

void test_symbol_table() {
    printf("[CTEST] Running symbol table tests...\n");
    test_symbol_table_tree();
    printf("[CTEST] Passed symbol table tests!\n");
}

void test_symbol_table_tree() {
    SymbolTable* table = symbol_table_new();

    // Test children creation
    SymbolTable* child = symbol_table_create_child(table, 0);
    assert(table->children_count == 1);
    assert(table->children_ptrs[0] == child);
    assert(table == child->parent);
    symbol_table_create_child(table, 0);
    symbol_table_create_child(table, 0);
    assert(table->max_children_count == 4);

    SymbolTable* childchild = symbol_table_create_child(child, 0);
    assert(childchild->parent == child);
    assert(childchild->parent->parent == table);

    symbol_table_free(table);
}

// When we allocate more than 1 child, we don't free correctly for the childchild
// Child is reallocated and therefore the pointer is invalid
// Is this an issue in the AST? We can't keep pointers to children in the AST like that,
// they will become invalid
// I could keep a vector of pointers to children. Now they are all separate and we 
// only care about the values of the pointers, not the actual memory location
// I could use a linked list for this too

// I want a tree structure, where the children are accessible from the node
// This should behave like a vector and allow me to add more children dynamically
// I also need to keep separate pointers to these children that live outside 
// So, it should be a vector of pointers to children. This will cause memory fragmentation,
// but that is unavoidable here anyway.