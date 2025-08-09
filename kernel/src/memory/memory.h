#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>
#include <c_lib.h>

typedef struct mem_block_t
{
    uint64_t baseAddr;
    uint64_t size;
} mem_block_t;


inline int MemCompareBlocksByAddr( const void* const a, const void* const b ) 
{
    const mem_block_t* const ia = a;
    const mem_block_t* const ib = b;
    if( ia->baseAddr < ib->baseAddr ) return -1;
    if( ia->baseAddr > ib->baseAddr ) return 1;
    return 0;
}

typedef struct mem_linear_arena_t
{
    mem_block_t memBlock;
    uint64_t used;
} mem_linear_arena_t;

// NOTE: https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-cc.adoc#integer-calling-convention
extern mem_linear_arena_t MemLinearArenaCreate( mem_block_t memBlock );
// NOTE: if no more space is left it will return NULL
extern uint8_t* MemLinearArenaAlloc( mem_linear_arena_t* const linearArena, uint64_t sizeInBytes );
extern void MemLinearReset( mem_linear_arena_t* const linearArena );
// NOTE: we just assume here
#define  MAX_PAGES_TOTAL (uint64_t) (uint16_t)(-1);

extern const uint64_t MAX_PHYSICAL_PAGE_SIZE;

typedef struct qk_ppm // Quakernel Physical Mem Manager
{
    mem_block_t* memRegions;
    uint64_t* pageBitMap;
    uint16_t pagesCount;
    uint8_t memRegionsCount;
} qk_ppm_t;

// NOTE: occupiedRegions MUST BE SORTED !
extern void QKEmitFreeAlignedMemBlocks( 
    const mem_block_t*      memRegions, 
    uint64_t                memRegionsCount, 
    const mem_block_t*      occupiedRegions, 
    uint64_t                occupiedRegionsCount, 
    mem_block_t* restrict   pFreeMemBlocks,
    uint64_t* restrict      pFreeMemBlocksCount
);

inline uint64_t AlignDown( uint64_t value, uint64_t alignment ) 
{
    return value & ~(alignment - 1);
}
inline uint64_t AlignUp( uint64_t value, uint64_t alignment ) 
{
    return (value + alignment - 1) & ~(alignment - 1);
}

#endif