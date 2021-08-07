// Various assignment operators

int main() {
    int x = 1;
    x += 5;
    x -= 3;
    x *= 10;
    x /= 5; // 6
    if (x != 6) {
        return 99;
    }
    int y = 0;
    y |= 11;
    y &= 1;
    y ^= 111;
    y <<= 1;
    y >>= 1;
    return y;
}