#include <string.h>

#include "uart.h"

const char UART_16550A_COMPAT[] = "ns16550a";
const char UART_KY_X1_COMPAT[] = "ky,pxa-uart";

uart_standard GetUartStdFromDtbCompat( const char* dtbCompat )
{
    if( strncmp( dtbCompat, UART_16550A_COMPAT, sizeof( UART_16550A_COMPAT ) ) == 0 )
    {
        return UART_STD_16550A;
    }
    // NOTE: according to the manufacturer it supports: 16550A & 16750
    if( strncmp( dtbCompat, UART_KY_X1_COMPAT, sizeof( UART_KY_X1_COMPAT ) ) == 0 )
    {
        return UART_STD_16550A;
    }
    return UART_STD_INVALID;
}

// Standard 16550 UART registers, indices for register offsets
typedef enum uart_16550A_register : uint8_t
{
    UART_16550A_REG_RBR     = 0x00, // Receiver Buffer Register (read)
    UART_16550A_REG_THR     = 0x00, // Transmitter Holding Register (write) — same as RBR
    UART_16550A_REG_IER     = 0x01, // Interrupt Enable Register
    UART_16550A_REG_IIR     = 0x02, // Interrupt Identification Register (read)
    UART_16550A_REG_FCR     = 0x02, // FIFO Control Register (write) — same as IIR
    UART_16550A_REG_LCR     = 0x03, // Line Control Register
    UART_16550A_REG_MCR     = 0x04, // Modem Control Register
    UART_16550A_REG_LSR     = 0x05, // Line Status Register
    UART_16550A_REG_MSR     = 0x06, // Modem Status Register
    UART_16550A_REG_SCR     = 0x07, // Scratch Register
} uart_16550A_register;

// NOTE: Standard 16550 UART will always contain 8 bits of data
// NOTE: we must read/write the entier register and mask the first byte

uint8_t Uart16550AReadRegister( 
    const uint8_t* const    baseAddr, 
    uint8_t                 regShift, 
    uint8_t                 regIOWidth, 
    uart_16550A_register    uartReg 
) {
    volatile const uint8_t* const reg = baseAddr + (uartReg << regShift);

    uint8_t dataOut;
    switch( regIOWidth )
    {
    case 1:
        dataOut = *(volatile const uint8_t* const)reg;
        break;
    case 2:
        dataOut = *(volatile const uint16_t* const)reg & 0xFF;
        break;
    case 4:
        dataOut = *(volatile const uint32_t* const)reg & 0xFF;
        break;
    case 8:
        dataOut = *(volatile const uint64_t* const)reg & 0xFF;
        break;
    default:
       break;
    }

    return dataOut;
}

void Uart16550AWriteRegister( 
    uint8_t* const          baseAddr, 
    uint8_t                 regShift, 
    uint8_t                 regIOWidth, 
    uart_16550A_register    uartReg, 
    uint8_t                 dataIn 
) {
    volatile uint8_t* const reg = baseAddr + (uartReg << regShift);

    switch( regIOWidth )
    {
    case 1:
        *(volatile uint8_t* const)reg = dataIn;
        break;
    case 2:
       *(volatile uint16_t* const)reg = dataIn;
        break;
    case 4:
        *(volatile uint32_t* const)reg = dataIn;
        break;
    case 8:
        *(volatile uint64_t* const)reg = dataIn;
        break;
    default:
    //panic
    break;
    }
}

bool UartTryReadByteFromRegister( const uart_port* const uartPort, char byte, uint8_t* const _out )
{   
    *_out = 0;

    if( UART_STD_16550A == uartPort->std )
    {
        uint8_t lsr = Uart16550AReadRegister(
            (uint8_t*) uartPort->baseAddr, uartPort->regShift, uartPort->regIOWidthInBytes, UART_16550A_REG_LSR);
        if( 0x01 == lsr )
        {
            uint8_t data = Uart16550AReadRegister(
                (uint8_t*) uartPort->baseAddr, uartPort->regShift, uartPort->regIOWidthInBytes, UART_16550A_REG_RBR);
            *_out = data;
            return true;
        }

        return false;
    }
    else
    {
        // PAnic
    }
    
}
bool UartTryWriteByteToRegister( const uart_port* uartPort, uint8_t dataIn )
{
    if( UART_STD_16550A == uartPort->std )
    {
        uint8_t lsr = Uart16550AReadRegister(
            (uint8_t*) uartPort->baseAddr, uartPort->regShift, uartPort->regIOWidthInBytes, UART_16550A_REG_LSR);
        if( 0x01 == lsr )
        {
            Uart16550AWriteRegister(
               (uint8_t*) uartPort->baseAddr, uartPort->regShift, uartPort->regIOWidthInBytes, UART_16550A_REG_THR, dataIn);
            return true;
        }

        return false;
    }
    else
    {
        // PAnic
    }
}