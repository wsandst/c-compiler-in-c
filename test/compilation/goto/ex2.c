// Jump to definition which is after the goto

int main() {
    int x = 0;
    goto end;
    x = 5;
    end:
    return x;
}
