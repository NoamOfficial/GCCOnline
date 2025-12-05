#ifndef IDE_DRIVER_H
#define IDE_DRIVER_H

// Plain types, no stdint
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long long u64;

// AH = 0 write, AH = 1 read
// lba = 48-bit LBA
// sectors = number of 512-byte sectors
// buffer = pointer to 16-bit word buffer (1 sector = 256 words)
void ide_driver(u8 ah, u64 lba, u32 sectors, u16* buffer);

#endif
