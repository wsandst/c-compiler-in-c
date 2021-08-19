// Switch breaks, fall through

int main() {
    int x = 4;
    switch (x) {
        case 4:
            switch (x) {
                case 1:
                    break;
                case 4:
                    x = 3;
            }
            break;
        case 3:
        case 2:
        case 1:
            return 10;
    }
    switch (x) {
        case 1:
            break;
        case 4:
            break;
        case 3:
        case 2:
            return 9; // Intended
    }
    return 1;
}
