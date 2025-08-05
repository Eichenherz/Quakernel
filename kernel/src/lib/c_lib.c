#include "c_lib.h"

void InsertionSort( uint8_t* arrayBase, uint64_t elemCount, uint64_t elemSize, CompareFunc pCompareFunc ) 
{
    uint8_t key[256];
    QK_CHECK( elemSize > ARRAY_LEN( key ) );

    for( uint64_t i = 1; i < elemCount; i++ ) 
    {
        memcpy( key, arrayBase + i * elemSize, elemSize );

        int64_t j = i - 1;
        for( ; j >= 0 && pCompareFunc( arrayBase + j * elemSize, key ) > 0; --j ) 
        {
            memcpy( arrayBase + (j + 1) * elemSize, arrayBase + j * elemSize, elemSize );
        }
        memcpy( arrayBase + (j + 1) * elemSize, key, elemSize );
    }
}