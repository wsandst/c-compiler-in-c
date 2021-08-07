// Casting

int main() {
    char x1 = 1;
    long int x2 = (long) (int) (short) x1;
    char x3 = (char) (int) (short) x2;
    char x4 = (char) x2;
    char x5 = 1;
    short x6 = 2;
    int x7 = 3;
    long x8 = x5 + x6 + x7;
    return x8;
}