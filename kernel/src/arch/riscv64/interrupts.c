#include "../interrupts.h"

#include <string.h>

__attribute__((noreturn))
void hcf() 
{
    for(;;) 
    {
        asm volatile ("wfi"); // Wait For Interrupt, saves power
    }
}

// TODO: These are very Ky X1 specific. Maybe add "boards" subfolder.
const char PLIC_COMPAT[] = "riscv,plic0";
const char CLINT_COMPAT[] = "riscv,clint0";
const char CPU_INTC_COMPAT[] = "riscv,cpu-intc";

const char GPIO_COMPAT[] = "ky,x1-gpio";

const char INTERRUPT_CTRL_PROP[] = "interrupt-controller";

interrupt_controller_type GetIntCtrlTypeFromDtbCompat( const char* dtbCompat )
{
#define CHECK_COMPAT_RETURN( compat_const, ret_val ) \
    if (strncmp( dtbCompat, compat_const, sizeof(compat_const)) == 0) { return ret_val; }

    CHECK_COMPAT_RETURN(PLIC_COMPAT, INC_PLIC)
    CHECK_COMPAT_RETURN(CLINT_COMPAT, INC_CLINT)
    CHECK_COMPAT_RETURN(CPU_INTC_COMPAT, INC_CPU_INC)
    CHECK_COMPAT_RETURN(GPIO_COMPAT, INC_GPIO)
    
    return INC_INVALID;
#undef CHECK_COMPAT_RETURN
}