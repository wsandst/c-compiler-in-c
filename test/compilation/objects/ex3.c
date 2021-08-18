// More advanced enum test

#include <stdio.h>

enum SWITCH_ENUM {
    A,
    B,
    C,
    D,
    E,
    F,
    G
};

typedef enum SWITCH_ENUM SWITCH_ENUM;

int switch_test(SWITCH_ENUM val) {
    switch (val) {
        case A:
        case B:
        case C:
            return 32;
        case E:
            return 90;
        default:
            return 3;
    }
}

int main() {
    return switch_test(A);// + switch_test(C) + switch_test(E) + switch_test(G) + switch_test(1);
}