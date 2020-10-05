#include <iostream>

#define ARRAYSIZE 10

int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    // Incorrectly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum += array[i];
    }

    // Incorrectly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum -= array[i];
    }

    return 0;
}

/* A function from NPB3.3.1/NPB3.3-SER/DC/jobcntl.c.
 * This is another kind of reduction that we should account for.
 * Its accumulator is conditionally incremented in each iteration
 */
uint32 NumberOfOnes(uint64 s){
   uint64 ob = MLB;
   uint32 i;
   uint32 nOnes;

   for ( nOnes = 0, i = 0; i < 64; i++ ) {
      if (s&ob) nOnes++;
      ob >>= 1;
   }
   return nOnes;
}
