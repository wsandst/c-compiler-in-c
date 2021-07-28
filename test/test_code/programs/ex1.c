// Fibonnaci program from internet

int fib() {
    int n = 11;
    int a = 0;
    int b = 1;
    int c;
    int i;
    if(n == 0)
        return a;
    for(i = 2; i <= n; i = i + 1)
    {
       c = a + b;
       a = b;
       b = c;
    }
    return b;
}

int main() {
    return fib();
}