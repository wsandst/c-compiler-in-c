

int main() {
    int x = 4;
    int y = 0;
    while (x > 0) {
        y = y + 1;
        x = x - 1;
    }
    return y; // Should be 4
}