// Scopes

int main() {
    int x = 4;
    {
        int y = 3;
        x = y;
    }
    int z = 2;
    return x;
}