// Break and continue

int main() {
    int i;
    for (i = 0; i < 100; i++)
    {
        continue;
    }
    for (int x = 0; x < 5; x++)
    {
        i++;
        if (x == 5) {
            break;
        }
    }
    
    return i;
}
