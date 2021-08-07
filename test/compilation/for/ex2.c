// Advanced for loop

int main() {
    int i;
    int y = 1;
    int z = 0;
    for (i = 10; i > y; i = i - y)
    {
        i = i - y;
        z = z + 1;
    }
    return i;
}