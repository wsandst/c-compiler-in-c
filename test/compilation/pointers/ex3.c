// Deref assignment

int main() {
    int ival = 5;
    int* iptr = &ival;
    *iptr = 1;
    float fval = 3.4;
    float* fptr = &fval;
    *fptr = 4.5;
    return ival + fval;
}