// From NPB3.3.1/NPB3.3-SER/DC/extbuild.c:129:3.
// total is identified as a likely reduction accumulator.

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
typedef  struct TYPE_11__   TYPE_5__ ;
typedef  struct TYPE_10__   TYPE_3__ ;
typedef  struct TYPE_9__   TYPE_2__ ;
typedef  struct TYPE_8__   TYPE_1__ ;

/* Type definitions */
typedef  scalar_t__ uint64 ;
struct TYPE_9__ {int /*<<< orphan*/  left; } ;
struct TYPE_11__ {int count; TYPE_2__ root; scalar_t__ memoryIsFull; } ;
struct TYPE_10__ {int inpRecSize; int nm; int nv; size_t numberOfChunks; int outRecSize; int nViewRows; TYPE_5__* tree; int /*<<< orphan*/  viewFile; int /*<<< orphan*/  logf; int /*<<< orphan*/  fileOfChunks; TYPE_1__* chunksParams; int /*<<< orphan*/  nd; int /*<<< orphan*/  selection; } ;
struct TYPE_8__ {int curChunkNum; void* chunkOffset; } ;

/* Variables and functions */
 int /*<<< orphan*/  InitializeTree (TYPE_5__*,int,int) ; 
 scalar_t__ MultiWayMerge (TYPE_3__*) ; 
 int /*<<< orphan*/  SelectToView (int /*<<< orphan*/ ,int /*<<< orphan*/ ,scalar_t__*,int /*<<< orphan*/ ,int,int) ; 
 int /*<<< orphan*/  TreeInsert (TYPE_5__*,scalar_t__*) ; 
 scalar_t__ WriteChunkToDisk (int,int /*<<< orphan*/ ,int /*<<< orphan*/ ,int /*<<< orphan*/ ) ; 
 scalar_t__ WriteViewToDiskCS (TYPE_3__*,int /*<<< orphan*/ ,int /*<<< orphan*/ *) ; 
 int /*<<< orphan*/  attrs ; 
 TYPE_3__* avp ; 
 void* chunkOffset ; 
 scalar_t__* currBuf ; 
 scalar_t__ currV ; 
 int /*<<< orphan*/  exit (int) ; 
 int /*<<< orphan*/  fprintf (int /*<<< orphan*/ ,char*) ; 
 int /*<<< orphan*/  fread (int /*<<< orphan*/ *,int,int,int /*<<< orphan*/ ) ; 
 int /*<<< orphan*/  fseek (int /*<<< orphan*/ ,long,int) ; 
 long ftell (int /*<<< orphan*/ ) ; 
 int iRec ; 
 int /*<<< orphan*/ * ib ; 
 size_t ibOffset ; 
 int iib ; 
 long inpfOffset ; 
 int /*<<< orphan*/  memcpy (int /*<<< orphan*/ ,int /*<<< orphan*/ *,int) ; 
 int nPart ; 
 int ncur ; 
 int nlst ; 
 int nreg ; 
 int nsgs ; 
 int /*<<< orphan*/  ordern ; 
 scalar_t__ prevV ; 
 scalar_t__ retCode ; 
 int /*<<< orphan*/  stderr ; 
 int total ; 

/******* the loop *******/

int main() {
    for (iib = 1; iib <= nsgs; iib++) {
        if (iib > 1)
            fseek(avp->viewFile, inpfOffset, 0);
        ;
        if (iib == nsgs)
            ncur = nlst;
        else
            ncur = nreg;
        fread(ib, ncur * avp->inpRecSize, 1, avp->viewFile);
        inpfOffset = ftell(avp->viewFile);
        for (ibOffset = 0 , iRec = 1; iRec <= ncur; iRec++) {
            memcpy(attrs, &ib[ibOffset], avp->inpRecSize);
            ibOffset += avp->inpRecSize;
            SelectToView(attrs, avp->selection, currBuf, avp->nd, avp->nm, avp->nv);
            currV = currBuf[2 * avp->nm];
            if (iib == 1 && iRec == 1) {
                prevV = currV;
                nPart = 1;
                InitializeTree(avp->tree, avp->nv, avp->nm);
                TreeInsert(avp->tree, currBuf);
            } else {
                if (currV == prevV) {
                    nPart++;
                    TreeInsert(avp->tree, currBuf);
                    if (avp->tree->memoryIsFull) {
                        avp->chunksParams[avp->numberOfChunks].curChunkNum = avp->tree->count;
                        avp->chunksParams[avp->numberOfChunks].chunkOffset = chunkOffset;
                        (avp->numberOfChunks)++;
                        if (avp->numberOfChunks >= 1024) {
                            fprintf(stderr, "Too many chunks were created.\n");
                            exit(1);
                        }
                        chunkOffset += (uint64)(avp->tree->count * avp->outRecSize);
                        retCode = WriteChunkToDisk(avp->outRecSize, avp->fileOfChunks, avp->tree->root.left, avp->logf);
                        if (retCode != 0) {
                            fprintf(stderr, "SharedSortAggregate: Write error occured.\n");
                            return retCode;
                        }
                        InitializeTree(avp->tree, avp->nv, avp->nm);
                    }
                } else {
                    if (avp->numberOfChunks && avp->tree->count != 0) {
                        avp->chunksParams[avp->numberOfChunks].curChunkNum = avp->tree->count;
                        avp->chunksParams[avp->numberOfChunks].chunkOffset = chunkOffset;
                        (avp->numberOfChunks)++;
                        chunkOffset += (uint64)(avp->tree->count * (4 * avp->nv + 8 * avp->nm));
                        retCode = WriteChunkToDisk(avp->outRecSize, avp->fileOfChunks, avp->tree->root.left, avp->logf);
                        if (retCode != 0) {
                            fprintf(stderr, "SharedSortAggregate: Write error occured.\n");
                            return retCode;
                        }
                    }
                    fseek(avp->viewFile, 0L, 2);
                    ;
                    if (!avp->numberOfChunks) {
                        avp->nViewRows += avp->tree->count;
                        retCode = WriteViewToDiskCS(avp, avp->tree->root.left, &ordern);
                        if (retCode != 0) {
                            fprintf(stderr, "SharedSortAggregate: Write error occured.\n");
                            return retCode;
                        }
                    } else {
                        retCode = MultiWayMerge(avp);
                        if (retCode != 0) {
                            fprintf(stderr, "SharedSortAggregate.MultiWayMerge: failed.\n");
                            return retCode;
                        }
                    }
                    InitializeTree(avp->tree, avp->nv, avp->nm);
                    TreeInsert(avp->tree, currBuf);
                    total += nPart;
                    nPart = 1;
                }
            }
            prevV = currV;
        }
    }
}