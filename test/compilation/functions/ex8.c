// Calling convention test

int f1(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a*8+b*7+c*6+d*5+e*4+f*3+f*2+h*1;
}

float f2(float a, float b, float c, float d, float e, float f, float g, float h, float i, int j) {
    return h*9+j*8+i*7+a*6+b*5+c*4+d*3+e*2+f*1;
}

float f3(float a, float b, int c, int d, int e, int f, float g, int h, float i, int j) {
    return a*6+b*5+c*4+d*3+e*2+f*1 + e*6+f*5+g*4+h*3+i*2+j*1;
}

float f4(float a, float b, int c, int d, int e, int f, float g, int h, 
        float i, int j, int k, int l, int m, int n, float o, int p, float q, 
        float r, float s, float t, float u, int w, float x) {
    return a*6+b*5+c*4+d*3+e*2+f*1 + e*6+f*5+g*4+h*3+i*2+j*1 + 
        k*6+l*5+g*4+m*3+n*2+o*1 +  p*6+q*5+r*4+s*3+t*2+u*1 + w*55 + x*81;
}


int main() {
    return f1(1, 2, 3, 4, 5, 6, 7, 8)
        + f2(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 1.0)
        + f3(1.1, 2, 3.3, 4, 5, 6.6, 7.7, 8, 9.9, 10)
        + f4(1.1, 2, 3.3, 4, 5, 6.6, 7.7, 8, 9.9, 10, 1.1, 2, 3.3, 4, 5, 6.6, 7.7, 8, 9.9, 10, 3.3, 3, 5.4);
}