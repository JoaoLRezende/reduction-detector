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
int reducee1(size_t array_size) {
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

int reduce7(int array[], size_t array_size) {
  int sum = 0;

  for (int i = 0; i < array_size; i++) {
    sum -= array[i];
  }
  return sum;
}
