// Calling convention test

int f1(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a*8+b*7+c*6+d*5+e*4+f*3+f*2+h*1;
}

float f2(float a, float b, float c, float d, float e, float f) {
    return a*6+b*5+c*4+d*3+e*2+f*1;
}

float f3(float a, float b, int c, int d, int e, int f, float g, int h, float i, int j) {
    return a*6+b*5+c*4+d*3+e*2+f*1 + e*6+f*5+g*4+h*3+i*2+j*1;
}


int main() {
    return f1(1, 2, 3, 4, 5, 6, 7, 8)
        + f2(1.1, 2.2, 3.3, 4.4, 5.5, 6.6) 
        + f3(1.1, 2.2, 3, 4, 5, 6, 7.7, 8, 9.9, 10);
}