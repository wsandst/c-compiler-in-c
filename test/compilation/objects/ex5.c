// Nested struct

struct TestStruct1 {
    int x;
    int y;
};

struct TestStruct2 {
    int x;
    int y;
    struct TestStruct1 s1;
    int z;
};

int main() {
    struct TestStruct2 t1;
    t1.x = 2;
    t1.y = 3;
    t1.z = 4;
    t1.s1.x = 5;
    t1.s1.y = 6;
    return t1.x + t1.y + t1.z + t1.s1.x + t1.s1.y;
}