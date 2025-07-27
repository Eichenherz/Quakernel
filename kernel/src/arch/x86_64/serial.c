#ifdef COM1_x86
#error("Already defined symbol")
#endif

#define COM1_x86 0x3f8

void PutCharSerial( char c )
{
    // NOTE: use volatile to prevent the complier form optimizing or reordering this 
    asm volatile (
        ".intel_syntax noprefix\n"
        "out dx, al\n"
        ".att_syntax prefix\n"
        :
        : "a"(c), "d"(COM1_x86)
    );
}

#undef COM1_x86