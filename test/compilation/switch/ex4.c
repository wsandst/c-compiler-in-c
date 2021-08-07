// Duffs device. Because why not?

int main() {
    int x = 0;
    int count = 60;
    {
        int n = (count + 7) / 8;
        switch (count % 8) {
        case 0: do { x++;
        case 7:      x++;
        case 6:      x++;
        case 5:      x++;
        case 4:      x++;
        case 3:      x++;
        case 2:      x++;
        case 1:      x++;
                } while ((--n) > 0);
        }
    }
    return x;
}