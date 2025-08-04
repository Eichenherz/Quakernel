#ifndef __C_LIB_H__
#define __C_LIB_H__

#include <stdint.h>

inline uint64_t AlignDown( uint64_t value, uint64_t alignment ) 
{
    return value & ~(alignment - 1);
}

inline uint64_t AlignUp( uint64_t value, uint64_t alignment ) 
{
    return (value + alignment - 1) & ~(alignment - 1);
}

#endif