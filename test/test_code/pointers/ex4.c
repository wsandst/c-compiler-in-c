// Void pointers, iteration

void* malloc(int size);
void free(void* ptr);

int main() {
    void* fptr = malloc(sizeof(int)*10);
    int* ptr = fptr;
    for (int i = 0; i < 10; i++)
    {
        *ptr = i;
        ptr++;
    }
    free(fptr);
    return *(ptr - 5);
}