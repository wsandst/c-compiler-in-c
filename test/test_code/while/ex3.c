// Do while loop

int main() {
    int y = 0;
    int x = 4;
    do {
        x = x - 1;
        y = y + 1;
    } while(x > 0);
    do {
        y = y + 1;
    } while(x == 100);
    return y;
}