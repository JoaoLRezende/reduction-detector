#include <stdio.h>

int arr[5] = { 1, 2, 3, 4, 5 };

int main() {
	int acc = 0;
	int accumulators[3];

	for (size_t i = 0; i < 3; ++i) {
		acc += arr[i];
		acc += arr[i];
		accumulators[2] += arr[i];
		accumulators[2] += arr[i];
	}

	for (int i = 0; i < 5; i++) {
		acc += arr[i];
		printf("%d ", acc);
	}

	for (int i = 0; i < 5; i++) {
		accumulators[0] = arr[i] + accumulators[0];
		printf("%d ", accumulators[0]);
	}
	printf("%d\n", acc);
	
	int i = 0;
	while (i < 10) {
		accumulators[1] += arr[i];
		i += 1;
	}
	
	do {
		accumulators[2] += arr[i];
		i += 1;
	} while (i < 10);
}

