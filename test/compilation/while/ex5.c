// Breaks

int main() {
    int x = 0;
    while (x < 10) {
        if (x > 6) {
            break;
        }
        x++;
    }
    while (1) {
        break;
        x++;
    }
    return x;
}