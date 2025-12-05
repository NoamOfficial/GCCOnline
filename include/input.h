void GetInput(void)
int input
_asm("mov al, 0xF4")
_asm("out 0x64, al")
_asm("in al, 0x60")
_asm("mov %input, al")
  return input
  
