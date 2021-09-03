// Global struct test

struct TestStruct {
    int x;
    int y;
    int z;
};

typedef struct TestStruct TestStruct;

TestStruct s;

int main() {
    s.x = 1;
    s.y = 2;
    s.z = 3;
    return s.x + s.y + s.z;
}