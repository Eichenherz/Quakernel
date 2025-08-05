#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>
#include <c_lib.h>

typedef struct mem_block_t
{
    uint64_t baseAddr;
    uint64_t size;
} mem_block_t;


inline int CompareMemBlocksByAddr( const void* const a, const void* const b ) 
{
    const mem_block_t* const ia = a;
    const mem_block_t* const ib = b;
    if( ia->baseAddr < ib->baseAddr ) return -1;
    if( ia->baseAddr > ib->baseAddr ) return 1;
    return 0;
}

// NOTE: we just assume here
const uint64_t MAX_PAGES_TOTAL = (uint16_t)(-1);

extern const uint64_t MAX_PHYSICAL_PAGE_SIZE;

typedef struct qk_ppm // Quakernel Physical Mem Manager
{
    mem_block_t memRegions[MAX_RESEVERD_PHYSICAL_MEMORY_REGIONS];
    uint64_t pageBitMap[ ( MAX_PAGES_TOTAL + 63 ) / 64 ];
    uint16_t pagesCount;
    uint8_t memRegionsCount;
} qk_ppm_t;

extern void QKInitPhysicalMemory();

// NOTE: occupiedRegions MUST BE SORTED !
extern void QKEmitFreeAlignedMemBlocks( 
    const mem_block_t*      memRegions, 
    uint64_t                memRegionsCount, 
    const mem_block_t*      occupiedRegions, 
    uint64_t                occupiedRegionsCount, 
    mem_block_t* restrict   pFreeMemBlocks,
    uint64_t* restrict      pFreeMemBlocksCount
);


#endif