#include <stdlib.h>

int main() {
    double* ptr = calloc(10, sizeof(double));;
    ptr[10] = 3.5;
    ptr[6] = 4.7;
    return ptr[10] + ptr[6];
}