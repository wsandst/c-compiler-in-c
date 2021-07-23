// Logical operators (and, or)

int main() {
    int and1 = 1 && 1;
    int and2 = 1 && 0;
    int and3 = 0 && 1;
    int and4 = 100 && 23;
    int or1 = 0 || 1;
    int or2 = 1 || 0;
    int or3 = 1 || 1;
    int or4 = 100 || 23;
    // This doesn't take order into account, which is pretty important
    return and1 + or1;
}