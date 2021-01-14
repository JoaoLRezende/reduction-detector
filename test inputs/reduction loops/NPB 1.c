// From NPB3.3.1/NPB3.3-SER/DC/adc.c:375:4.
// Both totalInBytes and nCubeTuples are identified as reduction accumulators.
// Seems right, though I'm not completely sure. TODO: examine this thing
// (in its context) more closely.

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
struct TYPE_2__ {int vidx; int vsize; } ;

/* Variables and functions */
 int dcdim ; 
 TYPE_1__* dcview ; 
 int /*<<< orphan*/  fprintf (int /*<<< orphan*/ ,char*,...) ; 
 size_t i ; 
 int idx ; 
 int j ; 
 size_t maxvn ; 
 size_t minvn ; 
 int nCubeTuples ; 
 int nViewDims ; 
 int totalInBytes ; 
 int /*<<< orphan*/  view ; 
 scalar_t__ vinc ; 

/******* the loop *******/

int main() {
    for (i = minvn; i < maxvn; i += vinc) {
        nViewDims = 0;
        fprintf(view, "Selection:");
        idx = dcview[i].vidx;
        for (j = 0; j < dcdim; j++)
            if ((idx >> j) & 1 == 1) {
                fprintf(view, " %lld", j + 1);
                nViewDims++;
            }
        fprintf(view, "\nView Size: %lld\n", dcview[i].vsize);
        totalInBytes += (8 + 4 * nViewDims) * dcview[i].vsize;
        nCubeTuples += dcview[i].vsize;
    }
}
