// Pass structs by value to functions

struct TestStruct {
    int x;
    int y;
};

typedef struct TestStruct TestStruct;

int test_func1(TestStruct s) {
    s.y = 10;
    return s.x;
}

int test_func2(int x1, int x2, int x3, int x4, int x5, int x6, int x7, TestStruct s) {
    s.y = 10;
    return s.x;
}

int test_func3(TestStruct s1, TestStruct s2, TestStruct s3) {
    s1.y = 4;
    s2.x = 5;
    s3.y = 3;
    return s1.x + s1.y + s2.x + s2.y + s2.x + s2.y;
}

int main() {
    TestStruct s;
    s.x = 1;
    s.y = 2;
    return test_func1(s) + test_func2(1, 2, 3, 4, 5, 6, 7, s) + test_func3(s, s, s) +
           s.x + s.y;
}