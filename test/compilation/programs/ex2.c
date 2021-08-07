// Insertion sort

void printf(char* string, int value);
void* malloc(int size);
void free(void* ptr);
int rand();

int count = 100;

void shift_element(int *arr, int *i) {
	int ival;
	for(ival = *i; i > arr && *(i-1) > ival; i--) {
		*i = *(i-1);
	}
	*i = ival;
}

void insertion_sort(int *arr, int len) {
	int *i = arr + len;
    int *last = i;
	for(i = arr + 1; i < last; i++)
		if(*i < *(i-1)) {
			shift_element(arr, i);
        }
}

void randomize_array(int* ptr, int n) {
    for(int i = 0; i < n; i++) {
		*ptr = rand() % 100;
		ptr++;
	}
}

int main() {
	int* arr_ptr = malloc(sizeof(int)*count);
    randomize_array(arr_ptr, count);
	
	insertion_sort(arr_ptr, count);
	
    int ret_val = *(arr_ptr+count/5);
    free(arr_ptr);
    return ret_val;
}