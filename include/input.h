static inline unsigned char GetInput(void) {
    unsigned char input;
    asm volatile (
        "inb $0x60, %0"   // read from keyboard data port
        : "=a" (input)    // output in AL -> input
        :                 // no inputs
        :                 // no clobbers
    );
    return input;
}
