// Struct assignment

struct TestStruct {
    int x; // 32 + 8 = 40
    int y;
};

typedef struct TestStruct TestStruct;

int test(TestStruct* s) {
    return (*s).x + (*s).y; // Deref s. int* x; *x;
}

int main() {
    TestStruct s;
    s.x = 5;
    s.y = s.x - 2;
    TestStruct* ptr = &s;
    (*ptr).x = (*ptr).y + 2;
    return test(&s);
}