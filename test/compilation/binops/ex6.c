// Less than, less than equals, greater than, greater then equals

int main() {
    if ((1 < 3) == 0)
        return 1;
    if ((3 < 1) != 0)
        return 1;
    if ((2 <= 2) == 0)
        return 1;
    if ((2 <= 3) == 0)
        return 1;
    if ((3 <= 2) == 0)
        return 1;
    if ((1 > 3) != 0)
        return 1;
    if ((3 > 1) == 0)
        return 1;
    if ((2 >= 2) == 0)
        return 1;
    if ((2 >= 3) != 0)
        return 1;
    if ((3 >= 2) == 0)
        return 1;
}