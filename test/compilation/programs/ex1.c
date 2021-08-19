// Iterative fibonnaci

int fib(int n) {
    int a = 0;
    int b = 1;
    int c;
    if (n == 0)
        return a;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

int main() {
    return fib(11);
}