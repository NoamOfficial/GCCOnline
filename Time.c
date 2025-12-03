// CMOS_TimeDriver.c
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// I/O port access
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// BCD → Binary
unsigned char bcd_to_bin(unsigned char val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

// CMOS Time Driver
// buffer[0] = sec, buffer[1] = min, buffer[2] = hour
// buffer[3] = day, buffer[4] = month, buffer[5] = year
void get_cmos_time(unsigned char* buffer) {
    // Wait until RTC not updating
    while (1) {
        outb(CMOS_ADDRESS, 0x0A);
        if (!(inb(CMOS_DATA) & 0x80)) break;
    }

    // Read CMOS
    outb(CMOS_ADDRESS, 0x00); buffer[0] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x02); buffer[1] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x04); buffer[2] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x07); buffer[3] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x08); buffer[4] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x09); buffer[5] = inb(CMOS_DATA);

    // Status B
    outb(CMOS_ADDRESS, 0x0B);
    unsigned char statusB = inb(CMOS_DATA);

    // Convert BCD → binary if needed
    if (!(statusB & 0x04)) {
        for (int i=0; i<6; i++) buffer[i] = bcd_to_bin(buffer[i]);
    }

    // 12-hour → 24-hour
    if (!(statusB & 0x02) && (buffer[2] & 0x80)) {
        buffer[2] = ((buffer[2] & 0x7F) + 12) % 24;
    }
}

