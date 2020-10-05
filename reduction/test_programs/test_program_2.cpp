#include <iostream>

#define ARRAYSIZE 10

int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + 2 + array[i] + sum + 1;
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = sum + 2 + array[i];
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + 2 + array[i] + sum;
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        x = sum + 3;
        sum = sum + 2 + array[i];
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = 2 + 3;
        sum = sum + 2 + array[i];
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = sum + sum;
        sum = sum + 2 + array[i];
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + 2 + array[i];
    }

    // Correctly unrecognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + sum + array[i] + sum;
    }

    // Correctly recognized.
    int i = 0;
    for(; i > ARRAYSIZE; --i){    
        sum = x + 2 + array[i] + sum;
    }

    return 0;
}
