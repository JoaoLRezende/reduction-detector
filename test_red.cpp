#include <iostream>

#define ARRAYSIZE 10
//PROBLEMA ATUAL EH BASICAMENTE: QUANDO ENCONTRO UM ACUMULADOR EM POTENCIAL, DEVO CONSEGUIR VERIFICAR SE APARECE APENAS UMA VEZ NO 
//RESTO DA EQUACAO, E DEPOIS VERIFICAR SE APARECE NOVAMENTE EM ALGUMA EQUACAO DENTRO DO FOR. MAS COMO GUARDAR ONDE DA EQUACAO/FOR QUE ENCONTREI O ACUMULADOR EM QUESTAO?
int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    for(int i = 0; i < ARRAYSIZE; i++){
        sum += array[i];//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){
        sum += sum + array[i];//nao
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){
        sum -= array[i];//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){
        x = x + array[i];//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){
        sum = x + array[i];//nao
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){    
        sum = x + sum + array[i];//sim
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//nao pode, pois x depende de sum, e nada alem de sum pode depender de sum
        x = sum;
        sum = sum + x;
    }//not done

    for(int i = 0; i < ARRAYSIZE; i++){
        sum = sum + array[i];//sim//done
        sum = sum - array[i];//sim//done
        sum = array[i] + sum;//sim//done
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//sim
        sum = sum * array[i];
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//sim, mesmo motivo do x
        sum = sum + array[i+1];
    }//done
    int j = 0;
    for(int i = 0; i < ARRAYSIZE; i++){//sim, mesmo motivo do x
        sum = sum + array[j];
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//sim
        sum = sum / array[i];
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//nao
        sum  = sum + sum;
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//sim, pois x replicado pode ser considerado uma string, a nao ser que ele receba sum em alguma parte do for
        sum  = sum + x;
    }//done

    for(int i = 0; i < ARRAYSIZE; i++){//nao
        sum = sum + array[i] + sum;
    }//done
/*
    for(){
        sum = fun(sum, array[i])
    }


    fun(sum, ele){
        ele = ele+ 10 + sum;
        sum = sum + ele;
    }*/

    return 0;
}
