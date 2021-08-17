// Static variables

static int x = 5;
char* str = "hmm";

int test() {
    static int t = 4;
    t++;
    return t;
}

int test2() {
    static char* s = "test";
    return s[0];
}

int main() {
    static int t = 3;
    test();
    test();
    t = test();
    test();
    return t + x + *str + test2();
}