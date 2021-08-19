// Simple struct test

struct TestStruct {
    int x;
    int y;
    int z;
};

int main() {
    struct TestStruct test;
    test.x = 2;
    test.y = 3;
    test.z = 4;
    return test.x + test.y + test.z;
}