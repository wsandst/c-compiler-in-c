// Floating point comparisons

int main() {
    if (3.2 < 3.0)
        return 1;
    if (3.0 > 3.2)
        return 1;
    if (!(3.0 < 3.2))
        return 1;
    if (!(3.2 > 3.0))
        return 1;
    if (3.0 != 3.0)
        return 1;
    if (2.0 == 3.0)
        return 1;
    if (!(3.0 >= 3.0))
        return 1;
    if (!(3.0 <= 3.0))
        return 1;
    //float f = 1.2;
    //f++;
    //f--;
    //++f;
    //--f;
    //if (f != 1.2) return 1;
    return 0;
}