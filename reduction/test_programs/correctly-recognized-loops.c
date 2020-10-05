#define ARRAYSIZE 10

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
    
    // Correctly recognized. // TODO: should this really be recognized?
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum + array[i];//sim//done
        sum = sum - array[i];//sim//done
        sum = array[i] + sum;//sim//done
    }//done

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum * array[i];
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum / array[i];
    }

    // Correctly recognized. ("sim, pois x replicado pode ser considerado uma string, a nao ser que ele receba sum em alguma parte do for")
    for(int i = 0; i < ARRAYSIZE; i++){
        sum  = sum + x;
    }
    
    // Correctly recognized. ("mesmo motivo do x")
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum + array[i+1];
    }
    
    // Correctly recognized. ("mesmo motivo do x")
    int j = 0;
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum + array[j];
    }

/*  // TODO
    for(){
        sum = fun(sum, array[i])
    }


    fun(sum, ele){
        ele = ele+ 10 + sum;
        sum = sum + ele;
    }*/
    
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

    return 0;
}
