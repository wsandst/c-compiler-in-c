// Function declarations

int putchar(int x);

int f2(int x);

int main() {
    putchar(121);
    putchar(97);
    putchar(121);
    putchar(10);
    return f2(7); 
}

int f2(int x) {
    return x+3;
}