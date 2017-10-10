/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include "mm.h"

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *sf_malloc(size_t size) {

    size_t asize;  /*ADJUSTED BLOCK SIZE*/
   // size_t extendsize;  /*EXTEND HEAP AMMOUNT WHEN NO FIT FOUND*/
    char *bp;

    /*IGNORE IF SIZE IS ZERO OR GREATER THAN PAGE SIZE*/
    if(size == 0 || size>PAGE){
        sf_errno =  EINVAL;
        return NULL;
    }

    //ADJUST THE BLACK SIZE AND ALIGNMENT
    if(size <= MEMALIGN)
        asize = 2*MEMROW + MEMALIGN;
    else
        asize = (((size+MEMALIGN-1)/MEMALIGN)*MEMALIGN) + (2*MEMROW);

    //SEARCH FREE LISTS FOR A FIT
    if((bp = find_fit(asize))!= NULL){
        place(bp, asize);
        return bp;
    }

    //NO FIT FOUND. EXTEND THE HEAP TO GET MORE MEMORY
    //extendsize = MAX(asize, CHUNKSIZE);
    //if((bp = extend_heap(extendsize)) == NULL)
        extend_heap();
        return NULL;
    place(bp, asize);
    return bp;


	return NULL;
}

void *extend_heap(){

    char *bp;
    size_t size = PAGE;

    if((long)(bp = sf_sbrk()) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0)); /*FREE BLOCK HEADER*/
    PUT(FTRP(bp), PACK(size, 0));   /*FREE BLOCK FOOTER*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));  /*NEW EPILOGUE HEADER*/

    /*COLEASE IF THE PREVIOUS BLOCK WAS FREE*/
    return coalesce(bp);

}

void *coalesce(void *bp, int mode){

size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
size_t

    /*mode IS 0 COLEASE WITH LOWER ADDRESS AND mode IS 1 COLEASE WITH HIGHER ADDRESS*/
    if(mode == 0){

    }


}

void *sf_realloc(void *ptr, size_t size) {
	return NULL;
}

void sf_free(void *ptr) {
	return;
}
