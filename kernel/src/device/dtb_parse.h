#ifndef __DTB_PARSE_H__
#define __DTB_PARSE_H__

#include <stdint.h>
#include <stdbool.h>

#include "memory.h"

extern void         ValidateDtb( const void* pDtb );
extern uint64_t     DtbGetSize( const void* pDtb );
extern void         DtbGetPhysicalMemory( 
                        const void* pDtb, 
                        uint64_t maxRamCount, 
                        mem_block_t* restrict pRam, 
                        uint64_t* restrict pRamCount 
                    );

#endif