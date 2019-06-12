#include <iostream>

#define ARRAYSIZE 10


int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + 2 + array[i] + sum + 1;//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = sum + 2 + array[i];//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + 2 + array[i] + sum;//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){    
        x = sum + 3;
        sum = sum + 2 + array[i];//nao
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = 2 + 3;
        sum = sum + 2 + array[i];//nao
    }//not

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = sum + sum;
        sum = sum + 2 + array[i];//nao
    }//not

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + 2 + array[i];//nao
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + sum + array[i] + sum;//nao
    }//done


    return 0;
}
