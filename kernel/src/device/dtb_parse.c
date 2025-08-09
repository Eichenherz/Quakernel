#include "dtb_parse.h"
#include "uart.h"
#include "../arch/interrupts.h"

#include "../lib/c_macros.h"

#include <libfdt.h>

const char DTB_DEVICE_STATUS_OKAY[] = "okay";
const char DTB_DEVICE_STATUS_OK[] = "ok";

typedef struct dtb_interrupt_request
{
    uint32_t deviceDtbPhandle;
    uint32_t intControllerDtbPhandle;
    uint16_t requestNum;
} dtb_interrupt_request;


// NOTE: it's common for kernels to treat missing status as "enabled"
static bool CheckOkFdtNodeStatus( const void* pDtb, int nodeOffset )
{
    const char* pStatus = (const char*)fdt_getprop( pDtb, nodeOffset, "status", NULL );
    if( !pStatus ) { return true; }

    return ( strncmp( pStatus, DTB_DEVICE_STATUS_OKAY, sizeof( DTB_DEVICE_STATUS_OKAY ) ) == 0 ) 
            || ( strncmp( pStatus, DTB_DEVICE_STATUS_OK, sizeof( DTB_DEVICE_STATUS_OK ) ) == 0 );
}
static int ParseNodeRegister( const void* pDtb, int nodeOffset, uint64_t* restrict addr, uint64_t* restrict size )
{
    // TODO: get the parent offset more efficiently
    int parentOffset = fdt_parent_offset( pDtb, nodeOffset );
    if( parentOffset < 0 )
    {
        return parentOffset;
    }
    int addrCells = fdt_address_cells( pDtb, parentOffset );
    if( addrCells < 0 )
    {
        return addrCells;
    }
    int sizeCells = fdt_size_cells( pDtb, parentOffset );
    if( sizeCells < 0 )
    {
        return sizeCells;
    }

    const uint32_t* pReg = (const uint32_t*) fdt_getprop( pDtb, nodeOffset, "reg", NULL );

    uint64_t sizeFieldOffset = 0;

    if( 0 == addrCells )
    {
        *addr = 0;
    }
    else 
    {
        uint64_t baseAddr = fdt32_to_cpu( pReg[0] );
        if( 2 == addrCells )
        {
            baseAddr = ( baseAddr << 32 ) | ( (uint64_t) fdt32_to_cpu( pReg[1] ) );
            *addr = baseAddr;

            sizeFieldOffset = 2;
        }
        else
        {
            *addr = baseAddr;
            sizeFieldOffset = 1;
        }
    }
    if( 0 == sizeCells )
    {
        *size = 0;
    }
    else 
    {
        uint64_t sz = fdt32_to_cpu( pReg[0 + sizeFieldOffset] );
        if( 2 == sizeCells )
        {
            sz = ( sz << 32 ) | ( (uint64_t) fdt32_to_cpu( pReg[1 + sizeFieldOffset] ) );
            *size = sz;
        }
        else
        {
            *size = sz;
        }
    }

    return 0;
}

void ValidateDtb( const void* pDtb )
{
    uint32_t totalSz = fdt_totalsize( pDtb );
    int errCode = fdt_check_full( pDtb, totalSz );
    if( errCode ) 
    {
        // print
        hcf();
    }
}

// TODO: err check all props
void ParseDtb( const void* pDtb )
{
    for( int currentNodeOffest = 0; currentNodeOffest >= 0; currentNodeOffest = fdt_next_node( pDtb, currentNodeOffest, NULL ) ) 
    {
        // TODO: we'll look maybe parse stuff that doesn't have the compat field, unitl then !
        const char* pCompatible = (const char*)fdt_getprop( pDtb, currentNodeOffest, "compatible", NULL );
        if( !pCompatible ) { continue; }

        const char* pIntCtrlTag = (const char*)fdt_getprop( pDtb, currentNodeOffest, "interrupt-controller", NULL );
        interrupt_controller_type intCtrlType = GetIntCtrlTypeFromDtbCompat( pCompatible );
        if( INC_INVALID != intCtrlType && pIntCtrlTag ) 
        { 
            uint64_t addr, size;
            int errCode = ParseNodeRegister( pDtb, currentNodeOffest, &addr, &size );
            if( errCode < 0 )
            {
                hcf();
            }
            
            int lenOrErrCodeHolder;
            const uint32_t* const pHandle = fdt_getprop( pDtb, currentNodeOffest, "phandle", &lenOrErrCodeHolder );
            if( !pHandle )
            {
                // err lenOrErrCodeHolder
            }

            interrupt_controller intCtl = {
                .baseAddr = addr,
                .regSize = size,
                .phandle = fdt32_to_cpu(*pHandle),
                .type = intCtrlType
            };

            continue;
        }

        uart_standard uartStd = GetUartStdFromDtbCompat( pCompatible );
        if( UART_STD_INVALID != uartStd ) 
        { 
            if( CheckOkFdtNodeStatus( pDtb, currentNodeOffest ) ) 
            {

                uint64_t addr, size;
                int errCode = ParseNodeRegister( pDtb, currentNodeOffest, &addr, &size );
                if( errCode < 0 )
                {
                    hcf();
                }
                // TODO: if addr or size == 0 means smth like device is disabled...

                // Len check
                int lenOrErrCodeHolder;
                const void* const pRegShift = fdt_getprop( pDtb, currentNodeOffest, "reg-shift", &lenOrErrCodeHolder );
                if( !pRegShift )
                {
                    // err lenOrErrCodeHolder
                }
                // assert len ?
                const void* const pRegIOWidth = fdt_getprop( pDtb, currentNodeOffest, "reg-io-width", &lenOrErrCodeHolder );
                if( !pRegIOWidth )
                {
                    // err lenOrErrCodeHolder
                }
                const uint32_t* const pHandle = fdt_getprop( pDtb, currentNodeOffest, "phandle", &lenOrErrCodeHolder );
                if( !pHandle )
                {
                    // err lenOrErrCodeHolder
                }

                uint32_t regShift = *(const uint32_t* const) pRegShift;
                uint32_t regIOWidth = *(const uint32_t* const) pRegIOWidth;
                uart_port serialComPort = {
                    .baseAddr = addr,
                    .size = size,
                    .phandle = fdt32_to_cpu(*pHandle),
                    .std = uartStd,
                    .regShift = fdt32_to_cpu( regShift ),
                    .regIOWidthInBytes = fdt32_to_cpu( regIOWidth )
                };
            }

            continue; 
        }
        
        if( CheckOkFdtNodeStatus( pDtb, currentNodeOffest ) )
        {
            int lenOrErrCodeHolder;
            const uint32_t* const pIntParent = fdt_getprop( pDtb, currentNodeOffest, "interrupt-parent", &lenOrErrCodeHolder );
            if( !pIntParent )
            {
                // err lenOrErrCodeHolder
            }

            int numInterruptsOrErrCodeHolder;
            const uint32_t* pInterrupts = fdt_getprop( pDtb, currentNodeOffest, "interrupts", &lenOrErrCodeHolder );
            if( !pInterrupts )
            {
                // err lenOrErrCodeHolder
            }
            const uint32_t* const pHandle = fdt_getprop( pDtb, currentNodeOffest, "phandle", &lenOrErrCodeHolder );
            if( !pHandle )
            {
                // err lenOrErrCodeHolder
            }
        }

    }
}