// From NPB3.3.1/NPB3.3-SER/DC/adc.c:121:7.
// lexp is identified as a likely reduction accumulator.
// TODO: is it? Examine this and its surrounding context in that source file.

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
typedef  struct TYPE_2__   TYPE_1__ ;

/* Type definitions */
struct TYPE_2__ {int dim; int* mlt; scalar_t__* exp; } ;

/* Variables and functions */
 int fct ; 
 TYPE_1__** fctlist ; 
 size_t genexp ; 
 int /*<<< orphan*/  lexp ;

/************* the loop *************/

int main() {
  int k;
  for (k = 0; k < fctlist[genexp]->dim; k++) {
    if (fctlist[genexp]->mlt[k] == 1)
      break;
    if (fct != fctlist[genexp]->mlt[k])
      continue;
    lexp -= fctlist[genexp]->exp[k];
    break;
  }
}
