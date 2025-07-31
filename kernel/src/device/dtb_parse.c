#include "dtb_parse.h"
#include "uart.h"

#include <dtc/libfdt/libfdt.h>


bool ValidateDtb(const void* pDtb)
{
    uint32_t totalSz = fdt_totalsize( pDtb );
    int errCode = fdt_check_full( pDtb, totalSz );
    if( errCode ) {
        // print
        hcf();
    }
}

const char DTB_DEVICE_STATUS_OKAY[] = "okay";
const char DTB_DEVICE_STATUS_OK[] = "ok";

// TODO: err check all props
// TODO: cache currentNodeOffest in order to get uarts on demand
// NOTE: uartNodeOffest = 0 to fidn the first
uart_port DtbGeNexttUartPort( const void* pDtb, int uartNodeOffest )
{
    int thisUartNodeOffset = -1;
    uart_standard uartStd = UART_STD_INVALID;

    for( ; uartNodeOffest >= 0; uartNodeOffest = fdt_next_node( pDtb, uartNodeOffest, NULL ) ) 
    {
        const char* pCompatible = (const char*)fdt_getprop( pDtb, uartNodeOffest, "compatible", NULL );
        if( !pCompatible ) { continue; }

        uartStd = GetUartStdFromDtbCompat( pCompatible );
        if( UART_STD_INVALID == uartStd ) { continue; }

        const char* pStatus = (const char*)fdt_getprop( pDtb, uartNodeOffest, "status", NULL );
        if( !pStatus ) { continue; }

        if( ( strncmp( pStatus, DTB_DEVICE_STATUS_OKAY, sizeof( DTB_DEVICE_STATUS_OKAY ) ) == 0 ) 
            || ( strncmp( pStatus, DTB_DEVICE_STATUS_OK, sizeof( DTB_DEVICE_STATUS_OK ) ) == 0 ) 
        ) {
            thisUartNodeOffset = uartNodeOffest;
            break;
        }
    }

    if( thisUartNodeOffset == -1 )
    {
        // Print
        hcf();
    }

    // TODO: get the parent offset more efficiently
    int parentOffset = fdt_parent_offset( pDtb, thisUartNodeOffset );
    if( parentOffset < 0 )
    {
        // Err
    }
    int addrCells = fdt_address_cells( pDtb, parentOffset );
    if( addrCells < 0 )
    {
        // Err
    }
    if( addrCells > 2 )
    {
        //print we're a 64 bits kernel so our addr space is 64bits, at most 2 FDT addrCells
        hcf();
    }
    int sizeCells = fdt_size_cells( pDtb, parentOffset );
    if( sizeCells > 2 )
    {
        //print we're a 64 bits kernel so our addr space is 64bits, at most 2 FDT sizeCells
        hcf();
    }
  
    const uint32_t* pReg = (const uint32_t*) fdt_getprop( pDtb, thisUartNodeOffset, "reg", NULL );
    // Len check
    uint8_t regShift = *(const uint8_t*) fdt_getprop( pDtb, thisUartNodeOffset, "reg-shift", NULL );
    uint8_t regIOWidth = *(const uint8_t*) fdt_getprop( pDtb, thisUartNodeOffset, "reg-io-width", NULL );

    uint64_t baseAddr = ( (uint64_t) ( fdt32_to_cpu( pReg[0] ) ) << 32 ) | ( (uint64_t) fdt32_to_cpu( pReg[1] ) );
    // NOTE: it's absurd for the size to exceed even unit32
    uint32_t size = fdt32_to_cpu( pReg[3] );

    uart_port serialComPort = {
        .baseAddr = baseAddr,
        .size = size,
        .std = uartStd,
        .regShift = fdt8_to_cpu( regShift ),
        .regIOWidthInBytes = fdt8_to_cpu( regIOWidth )
    };

    return serialComPort;
}