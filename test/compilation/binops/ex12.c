// Bitwise operators

int main() {
    int x = 100;
    int y = 1;
    return (((y | x) & 100) ^ 11 << 3) >> 2;
}