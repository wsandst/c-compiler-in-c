// Array of structs

typedef struct TestStruct {
    int x;
    int y;
    int z;
} TestStruct;

int main() {
    TestStruct arr[64];
    arr[0].x = 1;
    arr[3].x = 2;
    arr[3].y = 3;
    arr[3].z = 4;
    arr[4].y = 5;
    arr[63].z = 6;
    return arr[0].x + arr[3].y + arr[3].z + arr[4].y + arr[63].z;
}