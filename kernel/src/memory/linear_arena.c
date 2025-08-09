#include "memory.h"

#include <stddef.h>

mem_linear_arena_t MemLinearArenaCreate( mem_block_t memBlock )
{
    mem_linear_arena_t memLinearArena = {
        .memBlock = memBlock,
        .used = 0
    };

    return memLinearArena;
}
uint8_t* MemLinearArenaAlloc( mem_linear_arena_t* const linearArena, uint64_t sizeInBytes )
{
    if( ( linearArena->used + sizeInBytes ) > linearArena->memBlock.size )
    {
        return NULL;
    }

    uint64_t mem = linearArena->memBlock.baseAddr + linearArena->used;
    linearArena->used += sizeInBytes;
    return (uint8_t*) mem;
}
void MemLinearReset( mem_linear_arena_t* const linearArena )
{
    linearArena->used = 0;
}