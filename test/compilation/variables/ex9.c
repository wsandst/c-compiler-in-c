// Static variables

static int x = 5;

int test() {
    static int t = 4;
    t++;
    return t;
}

int main() {
    static int t = 3;
    test();
    test();
    t = test();
    test();
    return t + x;
}