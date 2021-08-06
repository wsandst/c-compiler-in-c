
int* g1;
float* g2;
long* g3;

int main() {
    int l1 = 1;
    float l2 = 3.14;
    int l3 = 2;
    g1 = &l1;
    g2 = &l2;
    g3 = &l3;
    return *g1 + *g2 + *g3;
}