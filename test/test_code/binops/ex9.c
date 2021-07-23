// Less than, less than equals, greater than, greater then equals

int main() {
    int lt1 = 1 < 3;
    int lt2 = 3 < 1;
    int lte1 = 2 <= 2;
    int lte2 = 2 <= 3; 
    int lte3 = 3 <= 2; 
    int gt1 = 1 > 3;
    int gt2 = 3 > 1;
    int gte1 = 2 >= 2;
    int gte2 = 2 >= 3; 
    int gte3 = 3 >= 2; 
    // This doesn't take order into account, which is pretty important
    return lt1 + lt2 + lte1 + lte2 + lte3 + gt1 + gt2 + gte1 + gte2 + gte3;
}