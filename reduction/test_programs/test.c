int main() {
	int arr[5] = {0, 1, 2, 3, 4};
	int sum = 0, x = 0;
	for (int i = 0; i < 5; i++) {
		sum = sum + x;
		sum = x + sum;
		sum += arr[i];
	}
}
