#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>

typedef struct mem_block_t
{
    uint64_t baseAddr;
    uint64_t size;
} mem_block_t;

// NOTE: we just assume here
const uint64_t MAX_RESEVERD_PHYSICAL_MEMORY_BLOCKS = 4;

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