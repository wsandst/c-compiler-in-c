// Struct padding test

struct TestStruct1 {
    long x; // 32 + 8 = 40
    int y; // 24
    long z; // 16
    int w; // 12
    char u; // 8
    long v; // 0
};

struct TestStruct2 {
    long x; // 24 + 8 = 32
    int y; // 16
    double z; // 8
    char u; // 0
};

int main() {
    struct TestStruct1 t;
    t.x = 1;
    t.y = 2;
    t.z = 3;
    t.w = 4;
    t.u = 5;
    t.v = 6;
    struct TestStruct2 t2;
    t2.x = 1;
    t2.y = 2;
    t2.z = 3;
    t2.u = 5;
    return sizeof(struct TestStruct1) + sizeof(struct TestStruct2) + t2.u + t2.z + t.v +
           t.y;
}