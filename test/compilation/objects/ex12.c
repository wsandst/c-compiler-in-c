// Global struct test

struct TestStruct {
    int x;
    int y;
};

typedef struct TestStruct TestStruct;

TestStruct s;

int main() {
    s.x = 1;
    s.y = 2;
    return s.x + s.y;
}