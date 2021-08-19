// Short circuiting

int g = 0;

int increment() {
    g++;
    return 1;
}

int main() {
    int x = (1 && increment() && 0 && increment());
    int y = (0 || increment() || increment());
    int z = (1 && (1 && increment() && increment()) && increment());
    int w = (1 && (1 || increment()));
    return g;
}
