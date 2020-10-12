#define ARRAYSIZE 10

int main(){
    int array[ARRAYSIZE] = {0, 1 , 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    int x = 0;

    /* From NPB3.3.1/NPB3.3-SER/IS/is.c.
     * Both R23 and T23 are incorrectly identified as reduction accumulators.
     * This loop in fact computes 2^(-23) and 2^23.
     * Note that the counter variable, i, doesn't appear in the loop of the body.
     * Also, each assignment involves no variable other than the potential accumulator itself.
     * Perhaps we should start checking those things.
     */
    double	R23 = 1.0, T23 = 1.0;
    for (int i=1; i<=23; i++)
    {
      R23 = 0.50 * R23;
      T23 = 2.0 * T23;
    }
    
    /* From NPB3.3.1/NPB3.3-SER/DC/adc.c:121:7.
     * lexp is identified as a likely reduction accumulator.
     * TODO: is it? Examine this and its surrounding context in that source file.
     */
    int k;
    for (k = 0; k < fctlist[genexp]->dim; k++) {
        if (fctlist[genexp]->mlt[k] == 1)
            break;
        if (fct != fctlist[genexp]->mlt[k])
            continue;
        lexp -= fctlist[genexp]->exp[k];
        break;
    }
    
    return 0;
}
