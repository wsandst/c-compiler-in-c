// Struct assignment

struct TestStruct {
    int x;
    int y;
};

typedef struct TestStruct TestStruct;

int main() {
    TestStruct t1;
    t1.x = 1;
    t1.y = 2;
    TestStruct t2;
    t2 = t1;
    t2.x = 3;
    (*&t1) = (*&t2);
    return t2.x + t2.y + t1.x + t1.y;
}