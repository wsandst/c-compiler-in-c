// Simple typedef
typedef int size_t;

int main() {
    typedef float size_t;
    size_t x = 5.7;
    return x + 0.4;
}