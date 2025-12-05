// UltimateOS Shell Skeleton
#include <input.h>
#include <pio.h>

// ---------------------- Structs ----------------------
typedef struct {
    unsigned char Command;
    unsigned short ObjectName;
} Global;

typedef struct {
    unsigned char Command;
    unsigned short ObjectName;
} Public;

typedef struct {
    unsigned char Command;
    unsigned short ObjectName;
} Private;

typedef struct {
    unsigned char Command;
    unsigned short ObjectName;
} CommandStruct;

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

// ---------------------- Global Variables ----------------------
GraphicalObject user1;
unsigned short* vgaBase = (unsigned short*)0xB8000; // VGA memory pointer
unsigned char color = 0x0F;                          // Default color

// ---------------------- Functions ----------------------

// Dump a GraphicalObject to VGA memory
void dumpStructToVGA(GraphicalObject* user, unsigned char color) {
    unsigned char* ptr = (unsigned char*)user;
    int size = sizeof(GraphicalObject);
    for (int i = 0; i < size; i++) {
        vgaBase[i] = (color << 8) | ptr[i];
    }
}

// Switch window function
void SwitchWindow(NewWindow* newWin) {
    // Backup current user/window
    GraphicalObject OldWindow = user1;

    // Copy new window into user1 (simulating active window)
    for (int i = 0; i < sizeof(GraphicalObject); i++) {
        ((unsigned char*)&user1)[i] = ((unsigned char*)newWin)[i];
    }

    // Dump active window to VGA
    dumpStructToVGA(&user1, color);
}

// Run a command
void RunCommand(int command, int p1, int p2, int p3, int p4, int p5, int p6) 
int cmdLen = sizeof(command)
    int input = command;
    int lba = 1;
    unsigned short buffer = 0x2000;

    // Example inline ASM for raw C
    ide_driver(1, 1, 4, buffer);
    while (command[cmdLen] != '\0') cmdLen++; // get command length

    for (int i = 0; i < length - cmdLen - 10; i++) { 
        // -10 to avoid overrun for startLBA+endLBA
        int match = 1;
        for (int j = 0; j < cmdLen; j++) {
            if (buffer[i+j] != (unsigned char)command[j]) {
                match = 0;
                break;
            }
        }

        if (match) {
            // Found command name at position i
            int startPos = i + cmdLen;          // immediately after command
            startLBA = buffer[startPos] | (buffer[startPos+1] << 8); // 2 bytes
            endLBA = 0;

            // Next 8 bytes = endLBA (little endian)
            for (int k = 0; k < 8; k++) {
                endLBA |= ((unsigned long long)buffer[startPos+2+k]) << (8*k);
                count = endLBA - startLBA;
                ide_driver(1, startLBA, count, buffer);
                _asm_(call far 0x2000:0)
            }

            // Done! You can break or continue if multiple commands
            break;
        }
    }
}





