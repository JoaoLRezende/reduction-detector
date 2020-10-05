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
