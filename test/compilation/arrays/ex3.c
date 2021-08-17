#include <stdlib.h>
#include <stdio.h>

int main() {
    int* ptr[4];
    ptr[1] = malloc(sizeof(int));
    ptr[1][1] = 5;

    char buf[64];
    snprintf(buf, 64, "%d", 5);
    return ptr[1][1] + atoi(buf);
}