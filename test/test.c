#include <stdio.h>
#include "../src/string_helpers.h"

int main() {
    printf("Test!\n");
    StrVector test = str_split("hello world", " ");
    return 0;
}