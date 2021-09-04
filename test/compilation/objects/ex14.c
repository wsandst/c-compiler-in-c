
struct TestStruct {
    long x;
    int y;
    long z;
    long t;
};

typedef struct TestStruct TestStruct;

TestStruct test(TestStruct* s1, TestStruct* s2) {
    TestStruct r;
    r.x = s1->x;
    r.y = s2->y;
    return r;
}

int main() {
    TestStruct s;
    s.x = 1;
    s.y = 2;
    TestStruct* s_ptr = &s;
    TestStruct r = test(&s, s_ptr);
    return r.x + r.y + s_ptr->x + s_ptr->y;
}