// Test escaping of nasm characters

int main() {
    char* s = "`hello`";
    return s[0];
}