#define ARRAYSIZE 10

int f(int sum, int next) {
    return sum + next;
}

int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        x = x + array[i];
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + sum + array[i];
    }
    
    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum * array[i];
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum / array[i];
    }

    // Correctly recognized. ("mesmo motivo do x")
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum + array[i+1];
    }
    
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = f(sum, array[i]);
    }
    
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
    
    // Correctly recognized.
    int i = 0;
    for(; i > ARRAYSIZE; --i){    
        sum = x + 2 + array[i] + sum;
    }
    
    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum += array[i];
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum -= array[i];
    }
    
    return 0;
}
