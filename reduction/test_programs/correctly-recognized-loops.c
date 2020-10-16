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

    int f(int sum, int next) {
        return sum + next;
    }
    
    for(int i = 0; i < ARRAYSIZE; i++){
        sum = f(sum, array[i])
    }
    
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
    
    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum += array[i];
    }

    // Correctly recognized.
    for(int i = 0; i < ARRAYSIZE; i++){
        sum -= array[i];
    }
    
    return 0;
}

/* external examples that can't be pasted here due to having references
   to many external identifiers:
   
    /* From NPB3.3.1/NPB3.3-SER/DC/adc.c:375:4.
     * Both totalInBytes and nCubeTuples are identified as reduction accumulators.
     * Seems right, though I'm not completely sure. TODO: examine this thing
     * (in its context) more closely.
     * /
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
    
    /* From NPB3.3.1/NPB3.3-SER/DC/extbuild.c:265:5.
     * nViewRows is identified as a likely reduction accumulator.
     * Seems right. TODO: be sure.
     * /
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
    
    /* From NPB3.3.1/NPB3.3-SER/DC/extbuild.c:129:3.
     * total is identified as a likely reduction accumulator.
     * /
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
