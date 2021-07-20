#include <stdio.h>

#include "string_helpers_test.h"
#include "tokenizer_test.h"
#include "test_symbol_table.h"

int main() {
    printf("[CTEST] Running all tests...\n");
    test_string_helpers();
    test_tokenizer();
    test_symbol_table();
    printf("[CTEST] Passed all tests!\n");
    return 0;
}