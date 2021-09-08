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
  struct {
    int *result_array[1];
  } result_struct = {{result_destination}};

  *result_struct.result_array[0] = 0;
  for (size_t i = 0; i < array_size; i++) {
    *result_struct.result_array[0] = array[i];
    printf("%d ", *result_struct.result_array[0]);
  }
}

int reduce12(int array[], size_t array_size) {
  int sum = 0;
  for (size_t i = 0; i < array_size; ++i) {
    sum += array[i];
  }
  sum += 1; // The value of sum isn't lost here. Thus, the reduction above
            // shouldn't be discarded.
  return sum;
}

int reduce13(int array[], size_t array_size) {
  int sum = 0;
  for (size_t i = 0; i < array_size; ++i) {
    sum += array[i];
  }
  sum = sum + 1; // The value of sum isn't lost here. Thus, the reduction above
                 // shouldn't be discarded.
  return sum;
}

void reduce14(int array[], size_t array_size, int *out) {
  // The possible accumulator of this loop shouldn't be regarded as apparently
  // abandoned, and thus this reduction loop shouldn't be discarded. The base of
  // its accumulating l-value is a reference to a local variable that is never
  // read after the loop, and thus the program could believe that it is
  // abandoned, and then discard the loop as a likely reduction. But that base
  // is actually a pointer to an external variable, which does properly
  // accumulate a value and might be accessed from outside this function.
  int *out_pointer = out;
  for (size_t i = 0; i < array_size; ++i) {
    *out_pointer += array[i];
  }
}

struct linked_list_node {
  int a_number;
  struct linked_list_node *next;
};
int reduce15(struct linked_list_node *linked_list) {
  int count = 0;
  struct linked_list_node *walker = linked_list;
  while (walker != NULL) {
    count++;
    walker = walker->next;
  }
  return count;
}

// The following is a trivial reduction.
int reduce16(int array[], size_t array_size) {
  int count = 0;
  for (size_t i = 0; i < array_size; i++) {
    if (array[i] > 5)
      count++;
  }
  return count;
}
