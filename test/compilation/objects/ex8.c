

struct TestStruct;

typedef struct TestStruct TestStruct;

struct TestStruct {
    int x; // 32 + 8 = 40
    int y;
};

int main() {
    TestStruct t;
    t.x = 4;
    return t.x;
}