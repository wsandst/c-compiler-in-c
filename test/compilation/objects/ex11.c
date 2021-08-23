// Returning structs by value

struct TestStruct {
    int x;
    int y;
};

typedef struct TestStruct TestStruct;

TestStruct test_func(int x, int y) {
    TestStruct s;
    s.x = x;
    s.y = y;
    return s;
}

int main() {
    TestStruct s = test_func(1, 2);
    s = test_func(3, 4);
    s.y = test_func(5, 6).y;
    return s.x + s.y;
}