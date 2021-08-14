// Ifdef, ifndef

#define VAL1
#define VAL2
#undef VAL2
#undef VAL1
#define VAL1

int main() {
    #ifdef VAL1
        #ifdef VAL2
            return 1;
        #endif
        #ifndef VAL2
            return 2; // Intended
        #endif
        return 3;
    #endif
    return 4;
}