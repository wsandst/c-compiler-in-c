// Merge sort

void printf(char* string, int value);
void* malloc(int size);
void free(void* ptr);
int rand();
 
int count = 100;

// function to sort the subsection a[i .. j] of the array a[]
void merge_sort(int i, int j, int* a, int* aux) {
    if (j <= i) {
        return;     // the subsection is empty or a single element
    }
    int mid = (i + j) / 2;

    // left sub-array is a[i .. mid]
    // right sub-array is a[mid + 1 .. j]
    
    merge_sort(i, mid, a, aux);     // sort the left sub-array recursively
    merge_sort(mid + 1, j, a, aux);     // sort the right sub-array recursively

    int pointer_left = i;       // pointer_left points to the beginning of the left sub-array
    int pointer_right = mid + 1;        // pointer_right points to the beginning of the right sub-array
    int k;      // k is the loop counter

    // we loop from i to j to fill each element of the final merged array
    for (k = i; k <= j; k++) {
        if (pointer_left == mid + 1) {      // left pointer has reached the limit
            *(aux + k) = *(a + pointer_right);
            pointer_right++;
        } else if (pointer_right == j + 1) {        // right pointer has reached the limit
            *(aux + k) = *(a + pointer_left);
            pointer_left++;
        } else if (*(a + pointer_left) < *(a + pointer_right)) {        // pointer left points to smaller element
            *(aux + k) = *(a + pointer_left);
            pointer_left++;
        } else {        // pointer right points to smaller element
            *(aux + k) = *(a + pointer_right);
            pointer_right++;
        }
    }

    for (k = i; k <= j; k++) {      // copy the elements from aux[] to a[]
        *(a+k) = *(aux+k);
    }
}

void randomize_array(int* ptr, int n) {
    for(int i = 0; i < n; i++) {
		*ptr = rand() % 100;
		ptr++;
	}
}

int main() {
    int* a = malloc(sizeof(int)*count);
    int* aux = malloc(sizeof(int)*count);

    randomize_array(a, count);

    merge_sort(0, count-1, a, aux);

    return *(a + count/6);
}