
#ifndef MM_H
#define MM_H
#include <stdlib.h>

#define WSIZE 2
#define DSIZE 4
#define MEMROW 8
#define MEMALIGN 16
#define PAGE 4096



#define IS_HEAP_START(hdrp)(hdrp==get_heap_start())
#define IS_HEAP_END(hdrp)(hdrp == get_heap_end())

#define GET_SF_HEADER(hdrp)((sf_header*)hdrp)
#define GET_SF_FOOTER(ftrp)((sf_footer*)ftrp)
#define GET_SF_FREE_HEADER(hdrp)((sf_free_header*)hdrp)

#define GET_BLOCK_SIZE(hdrp)(GET_SF_HEADER(hdrp)->block_size << 4)
#define SET_BLOCK_SIZE(hdrp,size)(GET_SF_HEADER(hdrp)->block_size = (size>>4))

#define GET_BLOCK_FOOTER_SIZE(hdrp)(GET_SF_FOOTER(hdrp)->block_size << 4)
#define SET_BLOCK_FOOTER_SIZE(hdrp,size)(GET_SF_FOOTER(hdrp)->block_size = (size>>4))

#define HDR2PAYLOAD(hdrp)((char*)hdrp+(SF_HEADER_SIZE/8))
#define PAYLOAD2HDR(plrp)((char*)plrp-(SF_HEADER_SIZE/8))

#define HDR2FTR(hdrp)((char*)(hdrp)+GET_BLOCK_SIZE(hdrp)-(SF_FOOTER_SIZE/8))
#define FTR2HDR(ftrp)((char*)ftrp-GET_BLOCK_SIZE(ftrp)+(SF_FOOTER_SIZE/8))

#define NEXTHDR(hdrp)((char*)hdrp+GET_BLOCK_SIZE(hdrp))
#define PREVHDR(hdrp)((char*)hdrp-(GET_BLOCK_FOOTER_SIZE((char*)hdrp-(SF_FOOTER_SIZE/8))))

/*SEARCHES FREE LIST FOR A FIT */
sf_free_header *find_fit(size_t size);

sf_free_header *find(int list, size_t size);

void *set_header_footer_allocblk(sf_free_header *bp, size_t size, size_t asize);

sf_free_header *extend_heap();

sf_free_header *coalesce(sf_free_header *bp, int mode);

sf_free_header *coalesce_helper(sf_free_header *hdr, sf_free_header *bp);

void remove_from_seglist(sf_free_header *bp);

void remove_list_helper(sf_free_header *hdr, int list);

void add_to_seglist(sf_free_header *bp);

void add_list_helper(sf_free_header *hdr, int list);

sf_free_header *split(sf_free_header *bp, size_t size);

int is_block_padded(void *ptr);


#endif