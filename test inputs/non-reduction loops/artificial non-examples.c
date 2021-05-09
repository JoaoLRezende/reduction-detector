#include <stdlib.h>

void not_reduce1(int array[], size_t array_size) {
  int an_integer_declared_outside_of_the_loop = 0;
  // This loop contains no possible accumulators. The base of its only
  // l-value is declared inside the loop.
  for (;;) {
    int a_local_array[10];
    a_local_array[an_integer_declared_outside_of_the_loop] = 80085;
  }
}
