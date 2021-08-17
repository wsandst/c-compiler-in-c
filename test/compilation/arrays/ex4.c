// Global ints

int g[4];

int test() {
    static int s[4];
    s[3] = 3;
    return s[3];
}

int main() {
    g[3] = 3;
    g[2] = 2;
    return g[2] + test();
}