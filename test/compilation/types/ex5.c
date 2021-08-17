// Advanced sizeof
#include <stdlib.h>

int main() {
    char* x = malloc(sizeof(*x)*5);
    x[4] = 'f';
    int arr[10] = {0};
    return x[4] + sizeof(x) + sizeof(*x) + sizeof(arr) / sizeof(arr[0]);
}