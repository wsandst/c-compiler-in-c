// Goto with same label names as variables

int main() {
    int x = 3;
    goto x;
    x = x + 1;
    x:
    return x;
}
