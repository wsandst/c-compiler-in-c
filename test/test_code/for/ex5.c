// Break and continue

int main() {
    int i;
    for (i = 0; i < 100; i = i + 1)
    {
        continue;
    }
    for (int x = 0; x < 5; x = x + 1)
    {
        i = i + 1;
        if (x == 5) {
            break;
        }
    }
    
    return i;
}
