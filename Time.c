#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

unsigned char cmos_read(unsigned char reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

unsigned char bcd_to_bin(unsigned char val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

void get_cmos_time(
    unsigned char* sec,
    unsigned char* min,
    unsigned char* hour,
    unsigned char* day,
    unsigned char* month,
    unsigned char* year
) {
    // Wait until RTC is not updating
    while(cmos_read(0x0A) & 0x80);

    *sec   = cmos_read(0x00);
    *min   = cmos_read(0x02);
    *hour  = cmos_read(0x04);
    *day   = cmos_read(0x07);
    *month = cmos_read(0x08);
    *year  = cmos_read(0x09);

    // Check BCD/binary mode
    unsigned char statusB = cmos_read(0x0B);
    if (!(statusB & 0x04)) { // BCD mode
        *sec   = bcd_to_bin(*sec);
        *min   = bcd_to_bin(*min);
        *hour  = bcd_to_bin(*hour);
        *day   = bcd_to_bin(*day);
        *month = bcd_to_bin(*month);
        *year  = bcd_to_bin(*year);
    }

    // Handle 12-hour format
    if (!(statusB & 0x02) && (*hour & 0x80)) { // 12h PM
        *hour = ((*hour & 0x7F) + 12) % 24;
    }
}
