// Fib recursion test
// Get wrong result, inspect assembly manually

int fib(int n) {
    if (n <= 1)
        return n;
    return fib(n-1) + fib(n-2);
}
 
int main() {
    return fib(6);
}