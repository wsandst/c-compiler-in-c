// Default

int main() {
    int x = 4;
    switch (x) {
        case 1:
        case 2:
        case 3:
            return 1;
        default:
            switch (x) {
                default:
                case 2:
                case 10:
                    return 1;
                case 12:
                    break;
            }
    }
    return 0;
}