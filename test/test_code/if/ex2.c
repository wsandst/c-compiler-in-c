// Nested ifs

int main() {
    int x = 0;
    if (x + 1) {
        if (x == 0) {
            return 1;
        }
        if (x == 0) {
            return 2;
        }
    }
    return 3;
}