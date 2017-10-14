
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
#define SET_BLOCK_SIZE(hdrp, size)(GET_SF_HEADER(hdrp)->block_size = (size>>4))

#define HDR2PAYLOAD(hdrp)((char*)hdrp+SF_HEADER_SIZE)
#define PAYLOAD2HDR(plrp)((char*)plrp-SF_HEADER_SIZE)

#define HDR2FTR(hdrp)((char*)hdrp+GET_BLOCK_SIZE(hdrp)-SF_FOOTER_SIZE)
#define FTR2HDR(ftrp)((char*)ftrp-GET_BLOCK_SIZE(ftrp)+SF_FOOTER_SIZE)

#define NEXTHDR(hdrp)((char*)hdrp+GET_BLOCK_SIZE(hdrp))
#define PREVHDR(hdrp)((char*)hdrp-(GET_BLOCK_SIZE((char*)hdrp-SF_FOOTER_SIZE)))

/*SEARCHES FREE LIST FOR A FIT */
sf_free_header *find_fit(size_t size);

void place(void *bp, size_t size);

void *extend_heap();

sf_free_header *coalesce(sf_free_header *bp, int mode);

sf_free_header *coalesce_helper(sf_free_header *hdr, sf_free_header *bp);

sf_free_header *find(int list, size_t size);

void remove_from_seglist(sf_free_header *bp);

void remove_list_helper(sf_free_header *hdr, int list);

void add_to_seglist(sf_free_header *bp);

#endif