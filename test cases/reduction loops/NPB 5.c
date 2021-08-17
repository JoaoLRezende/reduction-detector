/* From NPB3.3.1/NPB3.3-SER/IS/is.c:409.
 * This loop is annotated as an OpenMP reduction in the OpenMP
 * parallelization of that benchmark.
 */

/******* declarations to possibilitate parsing *******/

#define NULL ((void*)0)
typedef unsigned long size_t;  // Customize by platform.
typedef long intptr_t; typedef unsigned long uintptr_t;
typedef long scalar_t__;  // Either arithmetic or pointer type.
/* By default, we understand bool (as a convenience). */
typedef int bool;
#define false 0
#define true 1

/* Forward declarations */

/* Type definitions */

/* Variables and functions */
 int NUM_KEYS ; 
 int i ; 
 int /*<<< orphan*/  j ; 
 scalar_t__* key_array ; 


/******* the loop *******/

int main() {
    for( i=1; i<NUM_KEYS; i++ )
        if( key_array[i-1] > key_array[i] )
            j++;
}
