include <input.h>
typedef struct {
unsigned char Command
unsigned short ObjectName
} Global
typedef struct {
unsigned char Command
unsigned short ObjectName
} Public
typedef struct {
unsigned char Command
unsigned short ObjectName
} Private
typedef struct {
unsigned char Command
unsigned short ObjectName
} Command
typedef struct {
    char username[32];
    char inputBuffer[256];
    int cursorPosition;
} GraphicalObject;

GraphicalObject user1;

void dumpStructToVGA(GraphicalObject* user, , GraphicalObject* Color) {
    unsigned char* ptr = (unsigned char*)user; // point to the start of the struct
    int size = sizeof(GraphicalObject);          // get struct size
    int i;
    for (i = 0; i < size; i++) {
        vgaBase[i] = (color << 8) | ptr[i];    // dump each byte as a char to VGA
    }
}
void SwitchWindow(NewWindow*)
int oldWindowSizeof = sizeof(GraphicalObject)
int OldWindowptr = &GraphicalObject
int ptrsize = sizeof(OldWindowptr)
OldWindow = GraphicalObject
dumpStructToVGA(NewWindow, NewWindow* color)
return 0
   }
}



