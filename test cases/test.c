#include <stdio.h>

int arr[5] = { 1, 2, 3, 4, 5 };

int main() {
	int acc = 0;

	for (size_t i = 0; i < sizeof(arr)/sizeof(*arr); ++i) {
		acc += arr[i];
	}
	acc = 0;
	printf("%d\n", acc);
}

