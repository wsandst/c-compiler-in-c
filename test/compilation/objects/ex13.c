// Issue with passing of structs

struct TestStruct {
    int x;
    int y;
};

typedef struct TestStruct TestStruct;

int test(int x, TestStruct t) {
    return t.x + t.y;
}

int main() {
    TestStruct t;
    t.x = 1;
    t.y = 2;
    return test(1, t);
}