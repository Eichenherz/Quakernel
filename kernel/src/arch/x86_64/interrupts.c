// Halt and catch fire function.
void hcf() {
    asm volatile ("cli");
    for (;;) 
    {
        asm volatile ("hlt");
    }
}