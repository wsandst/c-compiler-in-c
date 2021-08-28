// Static char array test

extern void printf(char* fmt, ...);

char* get_str(int i) {
    static char* str_array[10] = {
        "strval1", "4strval2", "strval3", "strval4", "strval5",
        "strval6", "strval7",  "strval8", "strval9", "strval10",
    };
    return str_array[i];
}

int main() {
    return *get_str(1);
}