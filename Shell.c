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
void RunCommand(int command) {
    int input = command;
    int lba = 1;
    unsigned short buffer = 0xFFFF;

    // Example inline ASM for raw C
    ide-driver(1, 01, 4, buffer)
}





