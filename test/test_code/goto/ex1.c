// Standard goto

int main() {
    int x = 0;
    loop:
    if (x < 10) {
        x = x + 1;
        goto loop;
    }
    return x;
}