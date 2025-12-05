// --------------------------------------------------------------
//                UltimateOS Shell (32-bit segmentation)
// --------------------------------------------------------------
#include <input.h>
#include <pio.h>

// ---------------------- Structs ----------------------
typedef struct {
    char username[32];
    char inputBuffer[256];
    int cursorPosition;
} GraphicalObject;

typedef struct {
    char username[32];
    char inputBuffer[256];
    int cursorPosition;
} NewWindow;

// ---------------------- Globals ----------------------
GraphicalObject user1;
unsigned short* vgaBase = (unsigned short*)0xB8000;
unsigned char color = 0x0F;

// ---------------------- VGA Dump ----------------------
void dumpStructToVGA(GraphicalObject* obj, unsigned char color)
{
    unsigned char* p = (unsigned char*)obj;
    int size = sizeof(GraphicalObject);

    for (int i = 0; i < size; i++)
        vgaBase[i] = (color << 8) | p[i];
}

// ---------------------- Switch Window ----------------------
void SwitchWindow(NewWindow* newWin)
{
    // overwrite user1
    unsigned char* dst = (unsigned char*)&user1;
    unsigned char* src = (unsigned char*) newWin;

    for (int i = 0; i < sizeof(GraphicalObject); i++)
        dst[i] = src[i];

    dumpStructToVGA(&user1, color);
}

// =============================================================
//                    RAW COMMAND LOADER
// =============================================================
// MFT LAYOUT INSIDE SECTOR 1:
//    [commandName][2-byte startLBA][8-byte endLBA]
// =============================================================

void RunCommand(char* command)
{
    // This is where the MFT will be loaded (1 sector)
    unsigned char* buffer = (unsigned char*)0x2000;

    // read MFT (LBA1)
    ide_driver(1, 1, 1, buffer);

    // compute command length manually
    int cmdLen = 0;
    while (command[cmdLen] != 0)
        cmdLen++;

    int length = 512; // single sector

    for (int i = 0; i < length - cmdLen - 10; i++) {

        int ok = 1;
        for (int j = 0; j < cmdLen; j++) {
            if (buffer[i + j] != (unsigned char)command[j]) {
                ok = 0;
                break;
            }
        }

        if (ok) {
            // -------------------------------
            // Parse startLBA (2 bytes LE)
            // -------------------------------
            unsigned short startLBA =
                buffer[i + cmdLen] |
                (buffer[i + cmdLen + 1] << 8);

            // -------------------------------
            // Parse endLBA (8 bytes LE)
            // -------------------------------
            unsigned long long endLBA = 0;

            for (int k = 0; k < 8; k++)
                endLBA |= ((unsigned long long)buffer[i + cmdLen + 2 + k]) << (8 * k);

            unsigned long long count = endLBA - startLBA;

            // Load the entire command from disk into 0x2000
            ide_driver(1, startLBA, count, buffer);

            // -------------------------------
            // FAR CALL to loaded code
            // Segment = 0x2000 >> 4
            // Offset  = 0
            // -------------------------------

            asm volatile(
                "pushw %[seg]\n"
                "pushw $0x0000\n"
                "lret\n"
                :
                : [seg] "r" ((unsigned short)(0x2000 >> 4))
            );

            return;
        }
    }






