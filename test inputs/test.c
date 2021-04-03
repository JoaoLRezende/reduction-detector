#include <stdio.h>

int arr[5] = { 1, 2, 3, 4, 5 };

int main() {
	int acc = 0;
	for (int i = 0; i < 5; i++) {
		acc += arr[i];
		printf("%d\n", acc);
	}
	printf("%d\n", acc);
	
	int i = 0;
	while (i < 10) {
		acc += arr[i];
		i += 1;
	}
	
	do {
		acc += arr[i];
		i += 1;
	} while (i < 10);
}

