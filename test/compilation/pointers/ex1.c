// Simple pointer test

float test(int* x, int** z, float* f) {
    return *x + **z + *f;
}

int main() {
    int x = 10;
    long z = 40;
    long* zp = &z;
    float f = 3.4;
    return test(&x, &zp, &f);
}