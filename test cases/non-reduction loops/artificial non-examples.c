#include <stdlib.h>

void not_reduction1(int array[], size_t array_size) {
  int an_integer_declared_outside_of_the_loop = 0;
  // This loop contains no possible accumulators. The base of its only
  // assignment LHS is declared inside the loop.
  for (;;) {
    int a_local_array[10];
    a_local_array[an_integer_declared_outside_of_the_loop] = 80085;
  }
}

int not_a_reduction2(int array[], size_t array_size) {
  int result = 0;
  for (size_t i = 0; i < array_size; ++i) {
    result += array[i];
  }
  result = 0; // The value of result is lost here. Thus, the reduction above should be discarded.
  return result;
}
