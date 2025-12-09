// First, define basic integer types if not included
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// ------------------- SOCKET STRUCT -------------------
typedef struct {
    u32 DestAddress;
    u16 DestPort;
    u32 Data;
} Socket;

// ------------------- SOCKET FUNCTIONS -------------------
u32 GetData(Socket* s) {
    return s->Data;
}

void SendData(Socket* s, u32 newdata) {
    // For demonstration, just store the data in the socket
    s->Data = newdata;
}


