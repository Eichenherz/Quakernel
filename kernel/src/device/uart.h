#include <stdint.h>

typedef enum uart_standard : uint8_t
{
    UART_STD_INVALID,
    UART_STD_16550A
} uart_standard;

typedef struct uart_port 
{
    uint64_t baseAddr;         
    uint32_t size;              // Size of the entire MMIO region mapped for UART registers (in bytes)
    uart_standard std;        
    uint8_t regShift;           // How many bits to shift register index to get offset (e.g., 2 means registers spaced by 4 bytes)
    uint8_t regIOWidthInBytes;  // Width of registers in bytes (e.g., 1, 2, or 4)
} uart_port;

extern uart_standard GetUartStdFromDtbCompat( const char* dtbCompat );
extern bool UartTryReadByteFromRegister( const uart_port* const uartPort, char byte, uint8_t* const _out );
extern bool UartTryWriteByteToRegister( const uart_port* uartPort, uint8_t dataIn );
