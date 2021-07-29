
int main() {
    int x = 4;
    int y = 0;
    while (x > 0) {
        y++;
        x--;
    }
    return y; // Should be 4
}