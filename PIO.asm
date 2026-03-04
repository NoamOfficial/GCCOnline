// IDE I/O ports
#define IDE_DATA       0x1F0
#define IDE_ERROR      0x1F1
#define IDE_SECCOUNT   0x1F2
#define IDE_LBA_LOW    0x1F3
#define IDE_LBA_MID    0x1F4
#define IDE_LBA_HIGH   0x1F5
#define IDE_DEVICE     0x1F6
#define IDE_STATUS     0x1F7
#define IDE_COMMAND    0x1F7

// ATA streaming opcodes
#define ATA_CMD_READ_STREAM_EXT   0x3B
#define ATA_CMD_WRITE_STREAM_EXT  0x3C

#define SECTOR_WORDS   256  // 512 bytes / 2 bytes per word

// Wait until device ready: BSY=0, DRQ=1
static void ata_wait_ready(void) {
    unsigned char status;
    do {
        __asm__ volatile ("inb %%dx, %%al" : "=a"(status) : "d"(IDE_STATUS));
    } while ((status & 0x80) || !(status & 0x08));
}

// READ_STREAM_EXT: stream a sector from LBA into buffer
void READ_STREAM(unsigned int lba, unsigned short *buffer, unsigned int count) {
    // Set task file registers
    __asm__ volatile ("outb %0, %1" : : "a"(0xE0 | ((lba >> 24) & 0x0F)), "d"(IDE_DEVICE));
    __asm__ volatile ("outb %0, %1" : : "a"(count & 0xFF), "d"(IDE_SECCOUNT));
    __asm__ volatile ("outb %0, %1" : : "a"(lba & 0xFF), "d"(IDE_LBA_LOW));
    __asm__ volatile ("outb %0, %1" : : "a"((lba >> 8) & 0xFF), "d"(IDE_LBA_MID));
    __asm__ volatile ("outb %0, %1" : : "a"((lba >> 16) & 0xFF), "d"(IDE_LBA_HIGH));

    // Send READ_STREAM_EXT command
    __asm__ volatile ("outb %0, %1" : : "a"(ATA_CMD_READ_STREAM_EXT), "d"(IDE_COMMAND));

    ata_wait_ready();

    // Stream data words
    for (unsigned int i = 0; i < count; i++) {
        unsigned short data;
        __asm__ volatile ("inw %%dx, %%ax" : "=a"(data) : "d"(IDE_DATA));
        buffer[i] = data;
    }
}

// WRITE_STREAM_EXT: stream a sector from buffer to LBA
void WRITE_STREAM(unsigned int lba, unsigned short *buffer, unsigned int count) {
    // Set task file registers
    __asm__ volatile ("outb %0, %1" : : "a"(0xE0 | ((lba >> 24) & 0x0F)), "d"(IDE_DEVICE));
    __asm__ volatile ("outb %0, %1" : : "a"(count & 0xFF), "d"(IDE_SECCOUNT));
    __asm__ volatile ("outb %0, %1" : : "a"(lba & 0xFF), "d"(IDE_LBA_LOW));
    __asm__ volatile ("outb %0, %1" : : "a"((lba >> 8) & 0xFF), "d"(IDE_LBA_MID));
    __asm__ volatile ("outb %0, %1" : : "a"((lba >> 16) & 0xFF), "d"(IDE_LBA_HIGH));

    // Send WRITE_STREAM_EXT command
    __asm__ volatile ("outb %0, %1" : : "a"(ATA_CMD_WRITE_STREAM_EXT), "d"(IDE_COMMAND));

    ata_wait_ready();

    // Stream data words
    for (unsigned int i = 0; i < count; i++) {
        unsigned short data = buffer[i];
        __asm__ volatile ("outw %%ax, %%dx" : : "a"(data), "d"(IDE_DATA));
    }
}

// Bare-metal entry
void kmain(void) {
    unsigned short buffer[SECTOR_WORDS];

    while (1) {
        READ_STREAM(0, buffer, SECTOR_WORDS);   // read LBA 0
        // optional processing here
        WRITE_STREAM(0, buffer, SECTOR_WORDS);  // write it back
    }
}
