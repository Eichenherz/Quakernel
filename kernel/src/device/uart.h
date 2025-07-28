#include <stdint.h>

// Standard 16550 UART registers, indices for register offsets
typedef enum uart_16550_register 
{
    UART_16550_REG_RBR     = 0x00, // Receiver Buffer Register (read)
    UART_16550_REG_THR     = 0x00, // Transmitter Holding Register (write) — same as RBR
    UART_16550_REG_IER     = 0x01, // Interrupt Enable Register
    UART_16550_REG_IIR     = 0x02, // Interrupt Identification Register (read)
    UART_16550_REG_FCR     = 0x02, // FIFO Control Register (write) — same as IIR
    UART_16550_REG_LCR     = 0x03, // Line Control Register
    UART_16550_REG_MCR     = 0x04, // Modem Control Register
    UART_16550_REG_LSR     = 0x05, // Line Status Register
    UART_16550_REG_MSR     = 0x06, // Modem Status Register
    UART_16550_REG_SCR     = 0x07, // Scratch Register
} uart_16550_register;

typedef struct uart_port 
{
    uint64_t baseAddr;
    uint32_t size;
    uint16_t regShift;
    uint16_t regIOWidthInBytes;
} uart_port;