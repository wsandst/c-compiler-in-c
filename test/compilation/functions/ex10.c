// Simple variadic function defintions
#include <stdarg.h>
#include <stdio.h>

char* asm_addf(int x, char* format_string, ...) {
    va_list vl;
    va_start(vl, format_string);
    static char buf[256];
    vsnprintf(buf, 255, format_string, vl);
    va_end(vl);
    return buf;
}

int main() {
    char* str = asm_addf(1, "%d %d", 2, 3);
    return str == '3';
}