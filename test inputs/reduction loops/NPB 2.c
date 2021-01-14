// From NPB3.3.1/NPB3.3-SER/DC/extbuild.c:265:5.
// nViewRows is identified as a likely reduction accumulator.
// Seems right. TODO: be sure.

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
struct TYPE_2__ {int nRowsToRead; int nm; int outRecSize; int* mSums; int* checksums; int /*<<< orphan*/ * memPool; int /*<<< orphan*/  nv; int /*<<< orphan*/  nd; int /*<<< orphan*/  selection; int /*<<< orphan*/  inpRecSize; } ;

/* Variables and functions */
 int KeyComp (int /*<<< orphan*/ *,int /*<<< orphan*/ *,int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/  SelectToView (int /*<<< orphan*/ ,int /*<<< orphan*/ ,int /*<<< orphan*/ *,int /*<<< orphan*/ ,int,int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/ * aggrBuf ; 
 int* aggrmp ; 
 int /*<<< orphan*/  attrs ; 
 TYPE_1__* avp ; 
 int compRes ; 
 int /*<<< orphan*/ * currBuf ; 
 int* currmp ; 
 int /*<<< orphan*/  exit (int) ; 
 int /*<<< orphan*/  fprintf (int /*<<< orphan*/ ,char*) ; 
 int /*<<< orphan*/  fread (int /*<<< orphan*/ ,int /*<<< orphan*/ ,int,int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/  fseek (int /*<<< orphan*/ ,long,int) ; 
 long ftell (int /*<<< orphan*/ ) ; 
 int fwrite (int /*<<< orphan*/ *,int,int,int /*<<< orphan*/ ) ; 
 size_t i ; 
 int iRec ; 
 long inpfOffset ; 
 int /*<<< orphan*/  iof ; 
 int measbound ; 
 int /*<<< orphan*/  memcpy (int /*<<< orphan*/ *,int /*<<< orphan*/ *,int) ; 
 size_t mpOffset ; 
 int nOut ; 
 int nOutBufRecs ; 
 int nViewRows ; 
 int /*<<< orphan*/ * prevBuf ; 
 int /*<<< orphan*/  stderr ; 

/******* the loop *******/

main() {
    for (iRec = 1; iRec <= avp->nRowsToRead; iRec++) {
        fread(attrs, avp->inpRecSize, 1, iof);
        SelectToView(attrs, avp->selection, currBuf, avp->nd, avp->nm, avp->nv);
        if (iRec == 1)
            memcpy(aggrBuf, currBuf, avp->outRecSize);
        else {
            compRes = KeyComp(&currBuf[2 * avp->nm], &prevBuf[2 * avp->nm], avp->nv);
            switch (compRes) {
              case 1:
                memcpy(&avp->memPool[mpOffset], aggrBuf, avp->outRecSize);
                mpOffset += avp->outRecSize;
                nOut++;
                for (i = 0; i < avp->nm; i++) {
                    avp->mSums[i] += aggrmp[i];
                    avp->checksums[i] += nOut * aggrmp[i] % measbound;
                }
                memcpy(aggrBuf, currBuf, avp->outRecSize);
                break;
              case 0:
                for (i = 0; i < avp->nm; i++)
                    aggrmp[i] += currmp[i];
                break;
              case -1:
                fprintf(stderr, "PrefixedAggregate: wrong parent view order.\n");
                exit(1);
                break;
              default:
                fprintf(stderr, "PrefixedAggregate: wrong KeyComp() result.\n");
                exit(1);
                break;
            }
            if (nOut == nOutBufRecs) {
                inpfOffset = ftell(iof);
                fseek(iof, 0L, 2);
                ;
                if (fwrite(avp->memPool, nOut * avp->outRecSize, 1, iof) != 1) {
                    fprintf(stderr, "\n Write error from WriteToFile()\n");
                    return 1;
                }
                ;
                fseek(iof, inpfOffset, 0);
                ;
                mpOffset = 0;
                nViewRows += nOut;
                nOut = 0;
            }
        }
        memcpy(prevBuf, currBuf, avp->outRecSize);
    }
}
