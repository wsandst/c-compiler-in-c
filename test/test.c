#include <stdio.h>
#include "string_helpers_test.h"
#include "tokenizer_test.h"

int main() {
    printf("[TEST] Running all tests...\n");
    test_string_helpers();
    test_tokenizer();
    return 0;
}