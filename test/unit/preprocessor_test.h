#pragma once
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../../src/tokens.h"
#include "../../src/preprocess.h"
#include "../../src/util/file_helpers.h"

// Declarations
void test_preprocessor();
void test_preprocessor_table();

// Definitions
void test_preprocessor() {
    printf("[CTEST] Running preprocessor tests...\n");
    test_preprocessor_table();
    printf("[CTEST] Passed preprocessor tests!\n");
}

void test_preprocessor_table() {
    PreprocessorTable table = preprocessor_table_new();
    PreprocessorItem item;
    item.type = 4;
    item.name = "item1";
    preprocessor_table_insert(&table, item);
    item.type = 3;
    item.name = "item2";
    preprocessor_table_insert(&table, item);

    assert(preprocessor_table_lookup(&table, "item1")->type == 4);
    assert(preprocessor_table_lookup(&table, "item2")->type == 3);

    preprocessor_table_free(&table);
}

//int main() {
//test_preprocessor();
//}