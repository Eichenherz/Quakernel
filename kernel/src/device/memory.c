#include "memory.h"

#include <c_macros.h>
#include <c_lib.h>


void QKInitPhysicalMemory( const mem_block_t* pRamRegions, uint64_t ramRegionsCount )
{



}

void QKEmitFreeAlignedMemBlocks( 
    const mem_block_t*      memRegions, 
    uint64_t                memRegionsCount, 
    const mem_block_t*      occupiedRegions, 
    uint64_t                occupiedRegionsCount, 
    mem_block_t* restrict   pFreeMemBlocks,
    uint64_t* restrict      pFreeMemBlocksCount
) {
    // NOTE: use ramRegionsSize and not the max reserved
    for( uint64_t ramRegIdx = 0; ramRegIdx < memRegionsCount; ++ramRegIdx ) 
    {   
        const mem_block_t* const pCurrRamRegion = memRegions + ramRegIdx;
        const uint64_t regEnd = pCurrRamRegion->baseAddr + pCurrRamRegion->size;

        uint64_t currentStart = pCurrRamRegion->baseAddr;
        
        // NOTE: these are sorted
        for( uint64_t occupiedRamRegIdx = 0; occupiedRamRegIdx < occupiedRegionsCount; ++occupiedRamRegIdx )
        {
            const mem_block_t* const pCurrOccupiedRegion = occupiedRegions + occupiedRamRegIdx;
            const uint64_t occupiedStart = pCurrOccupiedRegion->baseAddr;
            const uint64_t occupiedEnd = pCurrOccupiedRegion->baseAddr + pCurrOccupiedRegion->size;
            
            // NOTE: these cases are not relevant for this algo
            if( occupiedEnd < currentStart || regEnd < occupiedStart ) continue;

            // NOTE: emit left-free block move the the next occupied and repeat
            if( currentStart < occupiedStart )
            {
                uint64_t alignedSize = AlignDown( occupiedStart - currentStart, sizeof( uint64_t ) );
                if( alignedSize )
                {
                    pFreeMemBlocks[*pFreeMemBlocksCount] = (mem_block_t) { 
                        .baseAddr = AlignUp( currentStart, sizeof( uint64_t ) ), .size = alignedSize
                    };
                    (*pFreeMemBlocksCount)++;
                }
            }

            currentStart = occupiedEnd;
            
        }
        // NOTE: emit left-over lol
        if( currentStart < regEnd )
        {
            uint64_t alignedSize = AlignDown( regEnd - currentStart, sizeof( uint64_t ) );
            if( alignedSize )
            {
                pFreeMemBlocks[*pFreeMemBlocksCount] = (mem_block_t) { 
                    .baseAddr = AlignUp( currentStart, sizeof( uint64_t ) ), .size = alignedSize
                };
                (*pFreeMemBlocksCount)++;
            }
        }
    }
}