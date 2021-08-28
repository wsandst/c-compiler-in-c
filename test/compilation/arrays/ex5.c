// Array initializer lists

int g_ints[4] = { 1, 2, 3, 4, 5 };
char* g_strs[4] = { "ab", "cd", "ef", "gh" };

int main() {
    int ints[5] = { 1, 2, 3, 4, 5 };
    long zeroes[10] = { 0 };
    char* strs[4] = { "ab", "cd", "ef", "gh" };
    static int s_ints[4] = { 1, 2, 3, 4, 5 };
    static char* s_strs[4] = { "ab", "cd", "ef", "gh" };
    if (zeroes[9] != 0) {
        return 1;
    }
    return ints[1] + strs[1][1] + s_ints[2] + s_strs[2][1] + g_ints[3] + g_strs[3][1];
}