#include <iostream>

#define ARRAYSIZE 10

int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum += sum + array[i]; // Note that sum is being added to sum.
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = x + array[i];
    }

    // Correctly unrecognized. ("x depende de sum, e nada alem de sum pode depender de sum")
    for(int i = 0; i < ARRAYSIZE; i++){
        x = sum;
        sum = sum + x;
    }
    
    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum  = sum + sum;
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum + array[i] + sum;
    }

    return 0;
}
