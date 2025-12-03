
#ifndef PCI_BAR_H
#define PCI_BAR_H

static inline unsigned long pciConfigReadDword(unsigned char bus,
                                               unsigned char slot,
                                               unsigned char func,
                                               unsigned char offset) {
    unsigned long address;
    unsigned long lbus  = (unsigned long)bus;
    unsigned long lslot = (unsigned long)slot;
    unsigned long lfunc = (unsigned long)func;

    address = ( (1UL << 31) | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) );

    outl(0xCF8, address);
    return inl(0xCFC);
}

// Reads all 6 BARs for a given PCI device
static inline void pciReadBARs(unsigned char bus,
                               unsigned char slot,
                               unsigned char func,
                               unsigned long *bar0,
                               unsigned long *bar1,
                               unsigned long *bar2,
                               unsigned long *bar3,
                               unsigned long *bar4,
                               unsigned long *bar5) {
    *bar0 = pciConfigReadDword(bus, slot, func, 0x10);
    *bar1 = pciConfigReadDword(bus, slot, func, 0x14);
    *bar2 = pciConfigReadDword(bus, slot, func, 0x18);
    *bar3 = pciConfigReadDword(bus, slot, func, 0x1C);
    *bar4 = pciConfigReadDword(bus, slot, func, 0x20);
    *bar5 = pciConfigReadDword(bus, slot, func, 0x24);
}

#endif
int LoadES(int ES, Int DI)
asm("mov es, %ES")
asm("mov di, %DI")
  #endif
  
