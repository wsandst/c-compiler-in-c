// Switch with non-existing value

int main() {
    int x = 10;
    switch (x) {
        case 3:
            return 3;
        case 2:
            return 2;
        case 1:
            return 1;
    }
    return 10;
}