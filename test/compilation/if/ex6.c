// Else ifs

int main() {
    int x = 4;
    if (x > 4) {
        return 1;
    }
    else if (x < 4) {
        return 2;
    }
    else if (x == 4) {
        if ((x + 2) == 3) {
            return 3;
        }
        else if (x == 3) {
            return 4;
        }
        else {
            return 5; // Intended
        }
        return 6;
    }
    else {
        return 7;
    }
    return 8;
}