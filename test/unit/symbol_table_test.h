#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../src/symbol_table.h"

void test_symbol_table();
void test_symbol_table_tree();
void test_symbol_table_vars();
void test_symbol_table_funcs();
void test_symbol_table_labels();
void test_symbol_table_objects();

void test_symbol_table() {
    printf("[CTEST] Running symbol table tests...\n");
    test_symbol_table_tree();
    test_symbol_table_vars();
    test_symbol_table_funcs();
    test_symbol_table_objects();
    test_symbol_table_labels();
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
    var.type.bytes = 1;
    var.type.ptr_level = 0;
    var.type.type = TY_INT;
    symbol_table_insert_var(table, var);
    assert(table->var_count == 1);
    var.name = "var2";
    var.type.bytes = 2;
    symbol_table_insert_var(table, var);
    var.name = "var3";
    var.type.bytes = 3;
    symbol_table_insert_var(table, var);
    assert(table->var_count == 3);
    assert(table->var_max_count == 4);

    // Lookup
    assert(symbol_table_lookup_var(table, "var1").type.bytes == 1);
    assert(symbol_table_lookup_var(table, "var2").type.bytes == 2);
    assert(symbol_table_lookup_var(table, "var3").type.bytes == 3);

    var.name = "var4";
    var.type.bytes = 4;
    symbol_table_insert_var(child, var);
    assert(symbol_table_lookup_var(child, "var4").type.bytes == 4);
    // Check going up a scope
    assert(symbol_table_lookup_var(child, "var1").type.bytes == 1);
    // symbol_table_lookup_var(child, "novar"); // This will correctly error!
    symbol_table_free(table);
}

void test_symbol_table_funcs() {
    SymbolTable* table = symbol_table_new();
    SymbolTable* child = symbol_table_create_child(table, 0);

    // Inserting
    Function func;
    func.name = "func1";
    func.def_param_count = 1;
    symbol_table_insert_func(table, func);
    assert(table->func_count == 1);
    func.name = "func2";
    func.def_param_count = 2;
    symbol_table_insert_func(table, func);
    func.name = "func3";
    func.def_param_count = 3;
    symbol_table_insert_func(table, func);
    assert(table->func_count == 3);
    assert(table->func_max_count == 4);

    // Lookup
    assert(symbol_table_lookup_func(table, "func1").def_param_count == 1);
    assert(symbol_table_lookup_func(table, "func2").def_param_count == 2);
    assert(symbol_table_lookup_func(table, "func3").def_param_count == 3);

    // Check going up a scope
    assert(symbol_table_lookup_func(child, "func1").def_param_count == 1);
    // symbol_table_lookup_var(child, "novar"); // This will correctly error!
    symbol_table_free(table);
}

void test_symbol_table_labels() {
    SymbolTable* table = symbol_table_new();
    SymbolTable* child = symbol_table_create_child(table, 0);
    SymbolTable* childchild1 = symbol_table_create_child(child, 0);
    SymbolTable* childchild2 = symbol_table_create_child(child, 0);
    ValueLabel label;
    // Test insert
    label.str_value = "0";
    symbol_table_insert_label(table, label);
    assert(table->label_count == 1);
    label.str_value = "1";
    symbol_table_insert_label(table, label);
    label.str_value = "2";
    symbol_table_insert_label(table, label);
    assert(table->label_count == 3);
    assert(table->label_max_count == 4);

    // Test recursively grabbing case labels
    label.str_value = "3";
    symbol_table_insert_label(child, label);
    label.str_value = "4";
    symbol_table_insert_label(childchild1, label);
    childchild2->is_switch_scope = true;
    label.str_value = "5";
    symbol_table_insert_label(childchild2, label);

    ValueLabel* labels = symbol_table_lookup_switch_case_labels(table);
    assert(strcmp(labels->str_value, "0") == 0);
    labels = labels->next;
    assert(strcmp(labels->str_value, "1") == 0);
    labels = labels->next;
    assert(strcmp(labels->str_value, "2") == 0);
    labels = labels->next;
    assert(strcmp(labels->str_value, "3") == 0);
    labels = labels->next;
    assert(strcmp(labels->str_value, "4") == 0);
    labels = labels->next;
    assert(labels == NULL);
    symbol_table_free(table);
}

void test_symbol_table_objects() {
    SymbolTable* table = symbol_table_new();
    SymbolTable* child = symbol_table_create_child(table, 0);

    // Inserting
    Object obj;
    obj.name = "obj1";
    obj.type = 1;
    symbol_table_insert_object(table, obj);
    assert(table->object_count == 1);
    obj.name = "obj2";
    obj.type = 2;
    symbol_table_insert_object(table, obj);
    obj.name = "obj3";
    obj.type = 3;
    symbol_table_insert_object(table, obj);
    assert(table->object_count == 3);
    assert(table->object_max_count == 4);

    // Lookup
    assert(symbol_table_lookup_object(table, "obj1", 1)->type == 1);
    assert(symbol_table_lookup_object(table, "obj2", 2)->type == 2);
    assert(symbol_table_lookup_object(table, "obj3", 3)->type == 3);

    obj.name = "obj4";
    obj.type = 4;
    symbol_table_insert_object(child, obj);
    assert(symbol_table_lookup_object(child, "obj4", 4)->type == 4);
    // Check going up a scope
    assert(symbol_table_lookup_object(child, "obj1", 1)->type == 1);
    // symbol_table_lookup_var(child, "novar"); // This will correctly error!
    symbol_table_free(table);
}

//int main() {
//test_symbol_table();
//}