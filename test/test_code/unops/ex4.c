// Increment, decrement

int main() {
    int x = 4;
    if ((x++) != 4) return 1;
    if ((x--) != 5) return 1;
    if ((++x) != 5) return 1;
    if ((--x) != 4) return 1;
    if (x != 4) return 1;
    return 0;
}