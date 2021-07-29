// Continues

int main() {
    int x = 0;
    int y = 0;
    while(x < 14) {
        x++;
        if ((x % 3) == 0) {
            continue;
            break;
        }
        y++;
    }
    return y;
}