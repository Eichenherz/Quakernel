#ifndef __C_LIB_H__
#define __C_LIB_H__

#include <stdint.h>

typedef int (*CompareFunc)(const void*, const void*);

extern void InsertionSort( uint8_t* arrayBase, uint64_t elemCount, uint64_t elemSize, CompareFunc pCompareFunc );

enum binary_prefixes : uint64_t {
    KiB = 1ULL << 10,   // 1024 bytes
    MiB = 1ULL << 20,   // 1,048,576 bytes
    GiB = 1ULL << 30,   // 1,073,741,824 bytes
    TiB = 1ULL << 40    // 1,099,511,627,776 bytes
};

#endif