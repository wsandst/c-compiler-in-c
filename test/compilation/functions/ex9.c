// Variadic function calling
void printf(char* format, ...);

int main() {
    printf("Variadic param test, %s %d %f %f %f %d\n", "very cool", 1, 3.3, 2.2, 1.1, 10);
    return 0;
}