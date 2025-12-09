typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long long u64;
typedef volatile u32   reg32;

#define AHCI_SECTOR_SIZE 512

#define PX_CLB 0x00
#define PX_CLBU 0x04
#define PX_FB 0x08
#define PX_FBU 0x0C
#define PX_IS 0x10
#define PX_CMD 0x18
#define PX_TFD 0x20
#define PX_CI 0x38

#define HBA_PxCMD_ST  (1<<0)
#define HBA_PxCMD_FRE (1<<4)
#define HBA_PxCMD_CR  (1<<15)
#define HBA_PxIS_TFES (1<<30)

#define FIS_TYPE_REG_H2D 0x27
#define ATA_CMD_READ_DMA_EX  0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

typedef struct { 
    reg32 clb,clbu,fb,fbu,is,ie,cmd,reserved0,tfd,sig,ssts,sctl,serr,sact,ci; 
} HBA_PORT;

typedef struct { 
    u16 flags,prdtl; 
    u32 prdbc,ctba,ctbau,reserved[4]; 
} HBA_CMD_HEADER;

typedef struct {
    u8 cfis[64]; 
    u8 acmd[16]; 
    u8 reserved[48];
    struct { u32 dba,dbau,reserved,dbc; } prdt_entry[1]; // single PRDT
} HBA_CMD_TABLE;

// ------------------- I/O -------------------
static inline void outl(u16 port,u32 val){ __asm__ volatile("outl %0,%1"::"a"(val),"d"(port)); }
static inline u32 inl(u16 port){ u32 ret; __asm__ volatile("inl %1,%0":"=a"(ret):"d"(port)); return ret; }

// ------------------- PCI -------------------
u32 pci_read(u8 bus,u8 slot,u8 func,u8 offset){ 
    u32 addr=(1<<31)|((u32)bus<<16)|((u32)slot<<11)|((u32)func<<8)|(offset&0xFC); 
    outl(0xCF8,addr); 
    return inl(0xCFC); 
}

u32 find_ahci_base(){
    for(int bus=0;bus<256;bus++)
        for(int slot=0;slot<32;slot++)
            for(int func=0;func<8;func++){
                u32 cc=pci_read((u8)bus,(u8)slot,(u8)func,0x08);
                u8 pi=(cc>>8)&0xFF, sub=(cc>>16)&0xFF, bc=(cc>>24)&0xFF;
                if(bc==0x01 && sub==0x06 && pi==0x01)
                    return pci_read((u8)bus,(u8)slot,(u8)func,0x24)&0xFFFFFFF0;
            }
    return 0;
}

// ------------------- PORT -------------------
HBA_PORT* find_first_port(u32 base){ 
    HBA_PORT* ports=(HBA_PORT*)(base+0x100); 
    for(int i=0;i<32;i++) if(ports[i].ssts & 0x0F) return &ports[i]; 
    return 0;
}

void ahci_start_port(HBA_PORT* port){
    while(port->cmd & HBA_PxCMD_CR){ __asm__ volatile("nop"); }
    port->cmd |= HBA_PxCMD_FRE|HBA_PxCMD_ST;
}

// ------------------- IRQ -------------------
volatile u8 ahci_command_done=0;

void ahci_irq_handler(){
    u32 base=find_ahci_base();
    HBA_PORT* port=find_first_port(base);
    if(!port) return;
    u32 is=port->is;
    if(is & HBA_PxIS_TFES){ /* handle error */ }
    port->is=is; // clear interrupt
    ahci_command_done=1; // signal completion
}

// ------------------- AHCI CMD -------------------
// funcname: 0=READ, 1=WRITE
void ahci_cmd(u32 funcname,u32 startlba,u32 count,u32 buffer_seg,u32 cmdlist_offset){
    u32 base=find_ahci_base();
    HBA_PORT* port=find_first_port(base);
    if(!port) return;
    ahci_start_port(port);

    HBA_CMD_HEADER* hdr=(HBA_CMD_HEADER*)(port->clb+cmdlist_offset);
    HBA_CMD_TABLE* tbl=(HBA_CMD_TABLE*)hdr->ctba;
    for(u32 i=0;i<8;i++) ((u32*)hdr)[i]=0;
    hdr->prdtl=1;

    tbl->prdt_entry[0].dba=buffer_seg;
    tbl->prdt_entry[0].dbau=0;
    tbl->prdt_entry[0].dbc=count*AHCI_SECTOR_SIZE-1|0x80000000;

    for(u32 i=0;i<64;i++) tbl->cfis[i]=0;
    tbl->cfis[0]=FIS_TYPE_REG_H2D;
    tbl->cfis[1]=0x80;
    tbl->cfis[2]=(funcname==0)?ATA_CMD_READ_DMA_EX:ATA_CMD_WRITE_DMA_EX;
    tbl->cfis[7]=(u8)count;
    tbl->cfis[8]=startlba&0xFF;
    tbl->cfis[9]=(startlba>>8)&0xFF;
    tbl->cfis[10]=(startlba>>16)&0xFF;
    tbl->cfis[12]=(startlba>>24)&0xFF;

    ahci_command_done=0;
    port->ci=1;

    while(!ahci_command_done); // wait for IRQ to signal completion
}



