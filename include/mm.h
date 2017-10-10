
#ifndef MM_H
#define MM_H
#include <stdlib.h>

#define WSIZE 2
#define DSIZE 4
#define CHUNKSIZE (1<<12)
#define MEMROW 8
#define MEMALIGN 16
#define PAGE 4096
#define EINVAL 22  /* Invalid argument */

#define MAX(x,y) ((x) > (y)? (x):(y))

/*PACK A SIZE AND ALLOCATED BIT INTO A WORD*/
#define PACK(size, alloc) ((size) | (alloc))

/*READ AND WRITE A WORDAT ADDRESS p*/
#define GET(p)  (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = val)

/*READ THE SIZE AND ALLOCATED FIELDS FROM ADDRESS p*/
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p)  (GET(p) & 0x1)

/*GIVEN BLOCK POINTER bp, COMPUTE ADDRESS OF ITS HEADER AND FOOTER*/
#define HDRP(bp)  ((char *)(bp) - WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/*GIVEN BLOCK POINTER bp, COMPUTE ADDRESS OF NEXT AND PREVIOUS BLOCK*/
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(BP)  ((char *)(bp) + GET_SIZE(((char *)(bp) - DSIZE)))

/*SEARCHES FREE LIST FOR A FIT */
void *find_fit(size_t size);

void place(void *bp, size_t size);

void *extend_heap();

void *coalesce(void *bp);

#endif