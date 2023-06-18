#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <malloc.h>

#include <time.h>

typedef struct
MemBlock
{
    int x;
    int y;
    struct MemBlock *pBlock;
} MemBlock;

#define HEAP_SZ         256 * 1024 * 1024

#define LARGE_ALLOC     864 * 1024
#define NITEMS          LARGE_ALLOC/sizeof(MemBlock)
#define NTIMES          100

#define GB              1024 * 1024 * 1024
int
main(void)
{
    HANDLE hLargeHeap = HeapCreate(0, HEAP_SZ, HEAP_SZ);
    if (hLargeHeap)
    {
        printf("Simulating %f GB.. \n", ((double)(LARGE_ALLOC * NTIMES)) / (GB));
        int i=0;
        LPVOID pArena = HeapAlloc(hLargeHeap, 0, LARGE_ALLOC);
        MemBlock *pRootBlock, *pTravers;
        clock_t start, end, accum = 0, accum2= 0, cSum;
        int sum=0;
    
        for (int j=0;
            j<NTIMES;
            j++)
        {
            pArena = HeapAlloc(hLargeHeap, 0, LARGE_ALLOC);
            pRootBlock = (MemBlock*) pArena;
            pTravers = pRootBlock;

            for (int i=0;
                i<(NITEMS-1);
                i++)
            {
                pTravers->x = rand()%10;
                pTravers->y = rand()%10;
                pTravers->pBlock = pTravers+1;
                pTravers++;
            }
            pTravers->pBlock = NULL;
            pTravers->x = rand()%10;
            pTravers->y = rand()%10;

            start = clock();
            pTravers = pRootBlock;
            for (pTravers=pRootBlock;
                pTravers->pBlock;
                pTravers = pTravers->pBlock)
            {
                sum+=pTravers->x;
                sum+= pTravers->y;
            }
            end = clock();
            accum += end - start;

            start = clock();
            HeapFree(hLargeHeap, 0, pArena);
            end = clock();
            accum2 += end - start;
        }
        cSum = accum+accum2;
        printf("Total time (Arena Allocation): %f secs\nSum: %f secs(%f %%)\nCleanup:%f secs(%f %%)\n",
            ((double)cSum)/CLOCKS_PER_SEC,
            ((double)accum)/CLOCKS_PER_SEC,
            ((double)accum/cSum) * 100.0F,
            ((double)accum2)/CLOCKS_PER_SEC,
            ((double)accum2/cSum) * 100.0F);
        accum = 0;
        accum2= 0;
        for (int j=0;
            j<NTIMES;
            j++)
        {
            pRootBlock = HeapAlloc(hLargeHeap, 0, sizeof(MemBlock));
            pTravers = pRootBlock;
            for (int i=0;
                i<(NITEMS-1);
                i++)
            {
                pTravers->x = rand()%10;
                pTravers->y = rand()%10;
                pTravers->pBlock = HeapAlloc(hLargeHeap, 0, sizeof(MemBlock));
                pTravers = pTravers->pBlock;
            }
            pTravers->pBlock = NULL;
            pTravers->x = rand()%10;
            pTravers->y = rand()%10;

            start = clock();
            for (pTravers=pRootBlock;
                pTravers->pBlock;
                pTravers = pTravers->pBlock)
            {
                sum+=pTravers->x;
                sum+= pTravers->y;
            }
            end = clock();
            accum += end -start;

            start = clock();
            for (pTravers=pRootBlock;
                pTravers->pBlock;
                )
            {
                pRootBlock = pTravers->pBlock;
                HeapFree(hLargeHeap, 0, (LPVOID)pTravers);
                pTravers = pRootBlock;
            }
            HeapFree(hLargeHeap, 0, (LPVOID)pTravers);
            end = clock();

            accum2 += end -start;
        }
        cSum = accum +accum2;
        printf("Total time (Heap-Block Allocation): %f secs\nSum: %f secs(%f %%)\nCleanup:%f secs(%f %%)\n",
            ((double)accum+accum2)/CLOCKS_PER_SEC,
            ((double)accum)/CLOCKS_PER_SEC,
            ((double)accum/cSum) * 100.0F,
            ((double)accum2)/CLOCKS_PER_SEC,
            ((double)accum2/cSum) * 100.0F);
        
        HeapDestroy(hLargeHeap);
    }
    else
    {
        printf("Failed to create heap of size %d", HEAP_SZ);
    }
    return 0;
}