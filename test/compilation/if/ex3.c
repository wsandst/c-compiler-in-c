// Nested ifs, scopes etc

int main() {
    int x = 5;
    if (x != 4) {
        int y = 3;
        y = y + 2;
        if (y == x) {
            x = 3;
        }
    }
    if (x == 3) {
        int y = 10;
        if (y == 10) {
            return 3;
        }
    }
    return 1;
}