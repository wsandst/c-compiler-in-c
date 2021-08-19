// Pointer operations

int putchar(char c);

int main() {
    char* str = "abcdHello world\n";
    int* str_int = (int*) str;
    ++str_int;
    str = (char*) str_int;
    str--;
    if (*str != 'H') {
        return 10;
    }
    while (*(++str) != 0) {
        putchar(*str);
    }
    --str;

    return *str;
}