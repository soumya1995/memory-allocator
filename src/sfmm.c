/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
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

/*NUMBER OF PAGES ALLOCATED*/
int pages = 0;

int sf_errno = 0;

void *sf_malloc(size_t size) {

    size_t asize;  /*ADJUSTED BLOCK SIZE*/
   // size_t extendsize;  /*EXTEND HEAP AMMOUNT WHEN NO FIT FOUND*/
    sf_free_header *bp;

    /*IGNORE IF SIZE IS ZERO OR GREATER THAN PAGE SIZE*/
    if(size == 0 || size>(PAGE*4)){
        sf_errno =  EINVAL;
        return NULL;
    }

    //ADJUST THE BLOCK SIZE AND ALIGNMENT
    if(size <= MEMALIGN)
        asize = 2*MEMROW + MEMALIGN;
    else
        asize = (((size+MEMALIGN-1)/MEMALIGN)*MEMALIGN) + (2*MEMROW);


    //SEARCH FREE LISTS FOR A FIT
    if((bp = find_fit(asize))!= NULL){


        if(GET_BLOCK_SIZE(bp)>asize) /*IF THE BLOCK SIZE OF THE FREE BLOCK IS GREATER THAN THE ADJUSTED SIZE SPLIT IT*/
            bp = split(bp, asize);

        return set_header_footer_allocblk(bp, size, asize);

    }

    /*WHEN THE ALLOCATOR CANNOT SATISY REQUEST AND RETURNS NULL*/
    sf_errno = ENOMEM;
	return NULL;
}

void *set_header_footer_allocblk(sf_free_header *bp, size_t size, size_t asize){



    sf_header *head_ptr = &(bp->header);
    sf_footer *ftr_ptr = GET_SF_FOOTER(HDR2FTR(head_ptr));

    /*SET FOOTER BLK SIZE*/
    SET_BLOCK_FOOTER_SIZE(ftr_ptr, asize);

    /*SET ALOCATED BITS TO 1*/
    head_ptr->allocated = 1;
    ftr_ptr->allocated = 1;

    if(size<16){

        head_ptr->padded = 1;
        ftr_ptr->padded = 1;
    }
    else if(size%16 == 0){

        head_ptr->padded = 0;
        ftr_ptr->padded = 0;
    }

    else if((size%16)!=0){

        head_ptr->padded = 1;
        ftr_ptr->padded = 1;
    }

    ftr_ptr->requested_size = size;

    return HDR2PAYLOAD(head_ptr);

}


sf_free_header *extend_heap(){

    char *bp;

    if(pages>4){
        sf_errno = ENOMEM;
        return NULL;
    }

    if((bp = sf_sbrk()) == (void*)-1)
        return NULL;

    pages++; //KEEP RECORD OF PAGES USED
    if(pages!=1){
        sf_free_header *hdr_free =GET_SF_FREE_HEADER(bp);
        SET_BLOCK_SIZE(&(hdr_free->header),PAGE_SZ);
        sf_free_header *hdr_prev = GET_SF_FREE_HEADER(PREVHDR(hdr_free));
        if(hdr_prev->header.allocated){ /*If previous block is allocated put hdr_free in free list*/
            add_to_seglist(hdr_free);
            return hdr_free;
        }
        else /*IF PREVIOUS BLOCK IS FREE COALESCE hdr_free with hdr_prev*/
            return coalesce(hdr_free, 0);

    }

    else{

        sf_free_header *hdr_free =GET_SF_FREE_HEADER(bp);
        SET_BLOCK_SIZE(&(hdr_free->header), PAGE_SZ);
        add_to_seglist(hdr_free);
        return hdr_free;
    }
}


sf_free_header *coalesce(sf_free_header *bp, int mode){


    sf_free_header *hdr = NULL;
    /*mode IS 0 COLEASE WITH LOWER ADDRESS */
    if(mode == 0){
        if(IS_HEAP_START(bp))
            return bp;
        hdr = GET_SF_FREE_HEADER(PREVHDR(bp));
        if(hdr->header.allocated)
            return bp;

        return coalesce_helper(hdr, bp);
    }

    /*mode IS 1 COLEASE WITH HIGHER ADDRESS*/
    else{
        if(IS_HEAP_END(bp))
            return bp;
        hdr = GET_SF_FREE_HEADER(NEXTHDR(bp));
        if(hdr->header.allocated){
            add_to_seglist(bp);
            return bp;
        }

        return coalesce_helper(bp, hdr);
    }

}

sf_free_header *coalesce_helper(sf_free_header *hdr, sf_free_header *bp){

    remove_from_seglist(hdr);
    remove_from_seglist(bp);


    sf_footer *newftrp = GET_SF_FOOTER(HDR2FTR(&(bp->header)));
    SET_BLOCK_SIZE(hdr,(GET_BLOCK_SIZE(&(hdr->header))+GET_BLOCK_SIZE(&(bp->header))));
    SET_BLOCK_FOOTER_SIZE(newftrp, (GET_BLOCK_SIZE(&(hdr->header))+GET_BLOCK_SIZE(&(bp->header))));

    add_to_seglist(hdr);

    return hdr;

}

void remove_from_seglist(sf_free_header *hdr){

   int size = GET_BLOCK_SIZE(hdr);
    if(size<=LIST_1_MAX)
        remove_list_helper(hdr, 0);
    else if(size<=LIST_2_MAX)
        remove_list_helper(hdr,1);
    else if(size<=LIST_3_MAX)
        remove_list_helper(hdr,2);
    else
        remove_list_helper(hdr,3);

}


void remove_list_helper(sf_free_header *hdr, int list){

    sf_free_header *head_ptr =(seg_free_list[list]).head;
    sf_free_header *node = NULL;

    if(head_ptr == hdr){ /*FIRST IN THE LIST*/
        head_ptr = hdr->next;
        (seg_free_list[list]).head = head_ptr;
    }

    else if(hdr->next == NULL) /*LAST IN THE LIST*/
        hdr->prev = NULL;

    else{
            for(node = head_ptr;node!=NULL;node = node->next){
                if(node == hdr){
                    (node->prev)->next = node->next;
                    (node->next)->prev = node->prev;
                }
        }
    }
}

void add_to_seglist(sf_free_header *hdr){


    int size = GET_BLOCK_SIZE(hdr);
    if(size<=LIST_1_MAX)
        add_list_helper(hdr, 0);
    else if(size<=LIST_2_MAX)
        add_list_helper(hdr,1);
    else if(size<=LIST_3_MAX)
        add_list_helper(hdr,2);
    else
        add_list_helper(hdr,3);
}

void add_list_helper(sf_free_header *hdr, int list){

    sf_free_header *head_ptr =(seg_free_list[list]).head;
    hdr->next = head_ptr;
    hdr->prev = NULL;
    (seg_free_list[list]).head = hdr;

}

sf_free_header *find_fit(size_t size){


    sf_free_header *blk;

    if(size<32)
        return NULL;
    int i=0;
    for (i = 0; i < FREE_LIST_COUNT; i++)//not sure yet
    {
        if((blk = find(i,size))!=NULL)
            return blk;
    }
    if(extend_heap()!=NULL){
        return find_fit(size);
    }
    return NULL;

}

sf_free_header *find(int list, size_t size){

    sf_free_header *head_ptr =(seg_free_list[list]).head;
    sf_free_header *node = NULL;

    for(node = head_ptr;node!=NULL;node = node->next){
        if(GET_BLOCK_SIZE(&(node->header))>=size){
            return node;
        }
    }

    return NULL;
}

sf_free_header *split(sf_free_header *bp, size_t size){


    size_t blk_size1 = size;
    size_t blk_size2 = 0;

    if(bp->header.allocated == 0)
        remove_from_seglist(bp);

    if((blk_size2 = (GET_BLOCK_SIZE(&(bp->header)) - size))>=32){

        /*FIRST FREE BLOCK*/
        sf_free_header *hdrp1 = GET_SF_FREE_HEADER(bp);
        SET_BLOCK_SIZE(&(hdrp1->header), blk_size1);
       // sf_footer *ftrp1 = GET_SF_FOOTER(HDR2FTR(&(hdrp1->header)));

       // SET_BLOCK_FOOTER_SIZE(ftrp1, blk_size1);

        /*SECOND FREE BLOCK*/
        sf_free_header *hdrp2 = GET_SF_FREE_HEADER(NEXTHDR(&(hdrp1->header)));
        SET_BLOCK_SIZE(&(hdrp2->header), blk_size2);
        //sf_footer *ftrp2 = GET_SF_FOOTER(HDR2FTR(&(hdrp2->header)));
        //SET_BLOCK_FOOTER_SIZE(ftrp2, blk_size2);

        //COALESING IF POSSIBLE AND PUTTING THE FREE BLOCK IN FREE LIST
        //coalesce(bp, 1);
        add_to_seglist(hdrp2);


    }

    return bp;

}



void *sf_realloc(void *ptr, size_t size) {

    /*CHECK FOR INVALID POINTER*/
    if(ptr == NULL)
        abort();
    if(ptr<get_heap_start() || get_heap_end()<(void*)(HDR2FTR(ptr)+(SF_FOOTER_SIZE/8)))
        abort();

    sf_header *hdr = GET_SF_HEADER(PAYLOAD2HDR(ptr));
    sf_footer *ftr = GET_SF_FOOTER(HDR2FTR(hdr));

    if(hdr->allocated == 0 || ftr->allocated == 0)
        abort();

    if((is_block_padded(ftr) == 1 && hdr->padded == 0) || (is_block_padded(ftr) == 0 && hdr->padded == 1) || (is_block_padded(ftr) == 1 && hdr->padded == 0) || (is_block_padded(ftr) == 0 && hdr->padded == 1)){
        abort();
    }

    if(hdr->padded != ftr->padded || hdr->allocated != ftr->allocated)
        abort();

    if(size == 0){

        sf_free(ptr);
        return NULL;
    }

    sf_header *ptr_hdr = GET_SF_HEADER(PAYLOAD2HDR(ptr));
    sf_footer *ptr_ftr = GET_SF_FOOTER(HDR2FTR(ptr_hdr));

    /*REALLOCATING TO A LARGER SIZE*/
    if(ptr_ftr->requested_size<size){

       void* new_ptr = sf_malloc(size);

       sf_header *new_ptr_hdr = GET_SF_HEADER(PAYLOAD2HDR(new_ptr));
       memcpy(HDR2PAYLOAD(new_ptr_hdr),HDR2PAYLOAD(ptr_hdr),ptr_ftr->requested_size);

       sf_free(ptr);
       return new_ptr;
    }

    /*REALLOCATING TO A SMALLER SIZE*/
    else if(ptr_ftr->requested_size>size){

        /*IF SPLITING MAKES SPLINTER*/
        if(ptr_ftr->requested_size-size<32){
            ptr_ftr->requested_size = size;
            return ptr;
        }

        /*IF SPLITTING DOESN'T FORM SPLINTER*/
        else{

            size_t asize;
            //ADJUST THE BLOCK SIZE AND ALIGNMENT
            if(size <= MEMALIGN)
                asize = 2*MEMROW + MEMALIGN;
            else
                asize = (((size+MEMALIGN-1)/MEMALIGN)*MEMALIGN) + (2*MEMROW);

             split(GET_SF_FREE_HEADER(ptr_hdr), asize);

             sf_free_header *next_free_hdr = GET_SF_FREE_HEADER(NEXTHDR(ptr_hdr));
             coalesce(next_free_hdr,1);

            return set_header_footer_allocblk(GET_SF_FREE_HEADER(ptr_hdr), size, asize);
        }
    }

    else
        return ptr;

}

void sf_free(void *ptr) {


    /*CHECK FOR INVALID POINTER*/
    if(ptr == NULL)
        abort();
    if(ptr<get_heap_start() || get_heap_end()<(void*)(HDR2FTR(ptr)+(SF_FOOTER_SIZE/8)))
        abort();

    sf_header *hdr = GET_SF_HEADER(PAYLOAD2HDR(ptr));
    sf_footer *ftr = GET_SF_FOOTER(HDR2FTR(hdr));

    if(hdr->allocated == 0 || ftr->allocated == 0)
        abort();

    if((is_block_padded(ftr) == 1 && hdr->padded == 0) || (is_block_padded(ftr) == 0 && hdr->padded == 1) || (is_block_padded(ftr) == 1 && hdr->padded == 0) || (is_block_padded(ftr) == 0 && hdr->padded == 1)){
        abort();
    }

    if(hdr->padded != ftr->padded || hdr->allocated != ftr->allocated)
        abort();

    hdr->allocated = 0;
    ftr->allocated = 0;
    coalesce(GET_SF_FREE_HEADER(hdr),1);

	return;
}

int is_block_padded(void *ptr){ //NOT SURE IF I SHOULD MAKE SEPARATE FOR HEADER N FOOTER

    if(((GET_SF_FOOTER(ptr)->requested_size)%16) == 0)
        return 0;
    else
        return 1;
}


