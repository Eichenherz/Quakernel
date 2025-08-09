#include <stdint.h>

typedef enum interrupt_controller_type : int8_t 
{
    INC_INVALID = 0,
    INC_PLIC = 1,
    INC_CLINT = 2,
    INC_CPU_INC = 3,
    INC_CLIC = 4,
    INC_GPIO = 5,
} interrupt_controller_type;

// TODO: add interrupts_extended ?
typedef struct interrupt_controller
{
    uint64_t baseAddr;
    uint64_t regSize;
    uint32_t phandle;
    // IRQ array and handlers
    interrupt_controller_type type;

} interrupt_controller;

extern interrupt_controller_type GetIntCtrlTypeFromDtbCompat( const char* compat );