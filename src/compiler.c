#include "tokens.h"
#include "file_helpers.h"

int main() {
    char* src = load_file_to_string("test/resources/test_source.c");

    Tokens tokens = tokenize(src);

    free(src);
    
    return 0;
}