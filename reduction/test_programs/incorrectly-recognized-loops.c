#include <iostream>

#define ARRAYSIZE 10

int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    /* From NPB3.3.1/NPB3.3-SER/IS/is.c.
     * Incorrectly recognized.
     * This loop computes 2^(-23) and 2^23.
     * Note that the counter variable, i, doesn't appear in the loop of the body.
     * Also, each assignment involves no variable other than the potential accumulator itself.
     * Perhaps we should start checking for those things.
     */
    double	R23 = 1.0, T23 = 1.0;
    for (i=1; i<=23; i++)
    {
      R23 = 0.50 * R23;
      T23 = 2.0 * T23;
    }
    
    return 0;
}
