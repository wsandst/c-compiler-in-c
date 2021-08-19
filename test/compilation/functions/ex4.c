
int test2() {
    int x = (10 * 2);
    return x / 2;
}

int test() {
    int x = 4;
    return (1 + x) - (x * test2());
}

int main() {
    return test();
}