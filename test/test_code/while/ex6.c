// Continues

int main() {
    int x = 0;
    int y = 0;
    while(x < 14) {
        x = x + 1;
        if ((x % 3) == 0) {
            continue;
            break;
        }
        y = y + 1;
    }
    return y;
}