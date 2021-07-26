// If else

// How to fix issue:
// Implement a way of parsing only a single statement
// Best done through a new function, parse_single_statement()
// which parse_statement() runs (and runs the next statment as well, unlike parse_single_statement)
// This is useful to support if else, or one line ifs and such.

int main() {
    return 1;
}

/*int main() {
    int x = 4;
    if (x == 4) {
        if (x != 4) {
            return 1;
        }
        else {
            return 2;
        }
    }
    else {
        return 3;
    }
    return 4;
}*/