// Deref and then address
#include <stdlib.h>

int main() {
    int* x = malloc(sizeof(int) * 4);
    *(x + 1) = 5;
    int* ptr1 = &(*(x + 1));
    x[3] = 3;
    int* ptr2 = &(x[3]);
    return *ptr2 + *ptr2;
}