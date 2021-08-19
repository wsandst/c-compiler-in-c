#include <stdio.h>

#include "vector_test.h"
#include "string_helpers_test.h"
#include "tokenizer_test.h"
#include "preprocessor_test.h"
#include "symbol_table_test.h"

int main() {
    printf("[CTEST] Running all unit tests...\n");
    test_vector();
    test_string_helpers();
    test_tokenizer();
    test_preprocessor();
    test_symbol_table();
    printf("[CTEST] Passed all unit tests!\n");
    return 0;
}