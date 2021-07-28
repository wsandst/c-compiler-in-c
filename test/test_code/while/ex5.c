// Breaks

int main() {
    int x = 0;
    while (x < 10) {
        if (x > 6) {
            break;
        }
        x = x + 1;
    }
    while(1) {
        break;
        x = x + 1;
    }
    return x;
}