__attribute__((noreturn))
void hcf() {
    for(;;) 
    {
        asm volatile ("wfi"); // Wait For Interrupt — optional, saves power
    }
}