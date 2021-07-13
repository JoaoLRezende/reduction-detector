#include <stdio.h>
#include <stdlib.h>

int f(int sum, int next) { return sum + next; }

int reduce1(int array[], size_t array_size) {
  int sum = 0;
  for (int i = 0; i < array_size; i++) {
    sum = sum + array[i];
  }
  return sum;
}

// An example in which an actual array appears in an array-subscript expression,
// rather than a pointer.
int reduce1point5(size_t array_size) {
  int arr[5] = {1, 2, 3, 4, 5};

  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum = sum + arr[i];
  }
  return sum;
}

int reduce2(int array[], size_t array_size) {
  int sum = 0;
  for (int i = 0; i < array_size; i++) {
    int old_sum = sum;
    old_sum += array[i];
    sum = old_sum;
  }
  return sum;
}

int reduce3(int array[], size_t array_size) {
  int sum = 0;
  for (int i = 0; i < array_size; i++) {
    sum = sum + array[i + 1];
  }
  return sum;
}

int reduce4(int array[], size_t array_size) {
  int sum = 0;
  for (int i = 0; i < array_size; i++) {
    sum = f(sum, array[i]);
  }
  return sum;
}

int reduce5(int array[], size_t array_size) {
  int sum = 0, i = 0;
  for (; i > array_size; --i) {
    sum = 2 + array[i] + sum;
  }
  return sum;
}

int reduce6(int array[], size_t array_size) {
  int sum = 0;
  for (int i = 0; i < array_size; i++) {
    sum += array[i];
  }
  return sum;
}

int reduce6point5(int array1[], int array2[], size_t arrays_size) {
  int sum_of_array1 = 0;
  int sum_of_both_arrays = 0;
	for (size_t i = 0; i < arrays_size; ++i) {
    sum_of_array1 += array1[i];
		sum_of_both_arrays += array1[i];
		sum_of_both_arrays += array2[i];
	}
  return sum_of_both_arrays;
}

int reduce7(int array[], size_t array_size) {
  int sum = 0;

  for (int i = 0; i < array_size; i++) {
    sum -= array[i];
  }
  return sum;
}

void reduce8(int array[], size_t array_size, int *accumulator) {
  for (size_t i = 0; i < array_size; i++) {
    *accumulator += array[i];
    printf("%d ", *accumulator);
  }
}

void reduce9(int array[], size_t array_size, int accumulators[]) {
  for (size_t i = 0; i < array_size; i++) {
    accumulators[0] += array[i];
    printf("%d ", accumulators[0]);
  }
}

struct values {
  int sum;
  int product;
};

void reduce10(int array[], size_t size, struct values *result) {
  result->sum = 0;
  for (size_t i = 0; i < size; ++i) {
    result->sum += array[i];
    printf("%d ", result->sum);
  }
}

void reduce11(int array[], size_t array_size, int *result_destination) {
  struct { int *result_array[1]; } result_struct = { { result_destination } };

  *result_struct.result_array[0] = 0;
  for (size_t i = 0; i < array_size; i++) {
    *result_struct.result_array[0] = array[i];
    printf("%d ", *result_struct.result_array[0]);
  }
}
