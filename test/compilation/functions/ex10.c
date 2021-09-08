// Simple variadic function defintions
#include <stdarg.h>
#include <stdio.h>

char* asm_addf(int x, int y, char* format_string, ...) {
    va_list vl;
    va_start(vl, format_string);
    static char buf[256];
    vsnprintf(buf, 255, format_string, vl);
    va_end(vl);
    return buf;
}

int main() {
    char* str = "3";
    char* output = asm_addf(1, 2, "%s %d", str, 1);
    printf("asm_addf: %s\n", output);
    return *output == '3' && *(output + 2) == '1';
}