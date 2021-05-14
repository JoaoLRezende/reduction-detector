#define ARRAYSIZE 10

/* A function from NPB3.3.1/NPB3.3-SER/DC/jobcntl.c.
 * This is another kind of reduction that we should account for.
 * Its accumulator is conditionally incremented in each iteration.
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

/******* the loop *******/

unsigned int NumberOfOnes(unsigned long long int s){
   unsigned long long int ob = 0x8000000000000000;
   unsigned int i;
   unsigned int nOnes;

   for ( nOnes = 0, i = 0; i < 64; i++ ) {
      if (s&ob) nOnes++;
      ob >>= 1;
   }
   return nOnes;
}
