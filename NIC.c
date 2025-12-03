// ultimateos_nic_network_full_ext.c
// Fully external NIC + TCP/IP + UDP + ARP stack for UltimateOS
// 32-bit, commands callable from ASM

typedef unsigned int  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

#define MAX_CONN 8
#define RXBUF_SIZE 1024
#define MAX_SEGMENTS 16
#define MAX_UNACK 16
#define TCP_WINDOW_SIZE 512
#define TCP_RETX_TIMEOUT 3000

#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IP  0x0800
#define IP_PROTO_UDP 17
#define IP_PROTO_TCP 6

// ------------------------ TCP Structures ------------------------
typedef struct {
    int active;
    u8 ip[4];
    u16 port;
    u32 seq;
    u32 ack;
    u32 expected_seq;
    u8 state; // 0=CLOSED,1=SYN_SENT,2=EST,3=FIN_WAIT,4=TIME_WAIT
    struct { u32 seq; u16 len; u8 data[512]; } segs[MAX_SEGMENTS];
    int seg_count;
    struct { u32 seq; u16 len; u8 data[512]; u32 sent_time; u8 acked; } unack[MAX_UNACK];
    int unack_count;
    u8 rx_buf[RXBUF_SIZE];
    u16 rx_start;
    u16 rx_end;
} tcp_conn_t;

tcp_conn_t connections[MAX_CONN];
u8 tcp_stop=0;

// ------------------------ NIC Hooks (Must Implement in UltimateOS) ------------------------
extern void NIC_SEND(u8* data,int len);
extern int NIC_RECV(u8* buf,int maxlen);
extern u8 MY_MAC[6];
extern u8 MY_IP[4];
extern u32 get_time_ms(void);

// ------------------------ Ethernet ------------------------
struct eth_frame { u8 dst[6]; u8 src[6]; u16 type; u8 payload[1500]; };

void eth_send(u8* dst,u16 type,u8* payload,u16 len){
    struct eth_frame f;
    for(int i=0;i<6;i++) f.dst[i]=dst[i], f.src[i]=MY_MAC[i];
    f.type=(type>>8)|(type<<8);
    for(int i=0;i<len;i++) f.payload[i]=payload[i];
    NIC_SEND((u8*)&f,len+14);
}

// ------------------------ ARP ------------------------
struct arp_req {
    u16 hwtype; u16 protype; u8 hwsize; u8 prosize;
    u16 opcode; u8 srcmac[6]; u8 srcip[4]; u8 dstmac[6]; u8 dstip[4];
};

u8 arp_cache_ip[4];
u8 arp_cache_mac[6];

extern void arp_send_request(u8* ip){
    u8 broadcast[6]={0xff,0xff,0xff,0xff,0xff,0xff};
    struct arp_req a;
    a.hwtype=0x0100; a.protype=0x0008;
    a.hwsize=6; a.prosize=4; a.opcode=0x0100; // request
    for(int i=0;i<6;i++){ a.srcmac[i]=MY_MAC[i]; a.dstmac[i]=0; }
    for(int i=0;i<4;i++){ a.srcip[i]=MY_IP[i]; a.dstip[i]=ip[i]; }
    eth_send(broadcast,ETH_TYPE_ARP,(u8*)&a,28);
}

void arp_handle(struct eth_frame* f){
    struct arp_req* a=(struct arp_req*)f->payload;
    if(a->opcode==0x0200){ // reply
        for(int i=0;i<4;i++) arp_cache_ip[i]=a->srcip[i];
        for(int i=0;i<6;i++) arp_cache_mac[i]=a->srcmac[i];
    }
}

// ------------------------ IPv4 ------------------------
struct ip_head { u8 ver_ihl; u8 tos; u16 totlen; u16 ident; u16 flags;
                u8 ttl; u8 proto; u16 checksum; u8 src[4]; u8 dst[4]; };

u16 ip_checksum(void* vdata,int length){
    u32 acc=0; u16* data=vdata;
    while(length>1){ acc+=*data++; length-=2; }
    if(length>0) acc+=*(u8*)data;
    while(acc>>16) acc=(acc&0xffff)+(acc>>16);
    return ~acc;
}

void ip_send(u8* dst_ip,u8 protocol,u8* payload,u16 payload_len){
    struct { struct ip_head h; u8 data[1500]; } p;
    p.h.ver_ihl=0x45; p.h.tos=0;
    p.h.totlen=((payload_len+20)>>8)|((payload_len+20)<<8);
    p.h.ident=0; p.h.flags=0; p.h.ttl=64; p.h.proto=protocol;
    for(int i=0;i<4;i++) p.h.src[i]=MY_IP[i], p.h.dst[i]=dst_ip[i];
    for(int i=0;i<payload_len;i++) p.data[i]=payload[i];
    p.h.checksum=0; p.h.checksum=ip_checksum(&p.h,20);
    eth_send(arp_cache_mac,ETH_TYPE_IP,(u8*)&p,20+payload_len);
}

// ------------------------ UDP ------------------------
struct udp_head { u16 src; u16 dst; u16 len; u16 checksum; };

extern void udp_send(u8* dst_ip,u16 src_port,u16 dst_port,u8* data,u16 len){
    struct { struct udp_head h; u8 data[1500]; } u;
    u.h.src=(src_port>>8)|(src_port<<8); u.h.dst=(dst_port>>8)|(dst_port<<8);
    u.h.len=((len+8)>>8)|((len+8)<<8); u.h.checksum=0;
    for(int i=0;i<len;i++) u.data[i]=data[i];
    ip_send(dst_ip,IP_PROTO_UDP,(u8*)&u,len+8);
}

// ------------------------ TCP Core ------------------------
extern void TCP_PROCESS_SEG(tcp_conn_t* c,u32 seq,u8* data,u16 len);
extern void tcp_handle_ack(tcp_conn_t* c,u32 ack);
extern void TCP_UPDATE(void);

void tcp_receive(struct ip_head* h,u8* data,u16 len){
    u16 src_port=(data[0]<<8)|data[1];
    u16 dst_port=(data[2]<<8)|data[3];
    u32 seq=(data[4]<<24)|(data[5]<<16)|(data[6]<<8)|data[7];
    u32 ack=(data[8]<<24)|(data[9]<<16)|(data[10]<<8)|data[11];
    u8 flags=data[13];
    for(int i=0;i<MAX_CONN;i++){
        tcp_conn_t* c=&connections[i];
        if(c->active && c->port==dst_port){
            if(flags & 0x10) tcp_handle_ack(c,ack); // ACK
            TCP_PROCESS_SEG(c,seq,data+20,len-20);
        }
    }
}

// ------------------------ IP Demux ------------------------
void ip_handle(struct eth_frame* f){
    struct ip_head* h=(struct ip_head*)f->payload;
    if(h->proto==IP_PROTO_UDP){ /* handle UDP */ }
    else if(h->proto==IP_PROTO_TCP) tcp_receive(h,(u8*)h+20,(h->totlen>>8|h->totlen<<8)-20);
}

// ------------------------ NIC Receive Loop ------------------------
extern void NIC_PROCESS(){
    while(1){
        u8 buf[1600]; int len=NIC_RECV(buf,1600);
        if(len<=0) return;
        struct eth_frame* f=(struct eth_frame*)buf;
        u16 type=(f->type>>8)|(f->type<<8);
        if(type==ETH_TYPE_ARP) arp_handle(f);
        else if(type==ETH_TYPE_IP) ip_handle(f);
    }
}

// ------------------------ External TCP Commands for ASM ------------------------
extern int TCP_OPEN(u8* ip,u16 port);
extern void TCP_SEND(int conn_id,u8* data,u16 len);
extern int TCP_RECEIVE(int conn_id,u8* outbuf,u16 maxlen);
extern void TCP_CLOSE(int conn_id);
extern void STOPPKT(void);
extern void PUSHS(int conn_id,u8* data,u16 len);
extern int POPS(int conn_id,u8* outbuf,u16 maxlen);
extern void PING(u8* ip);

// ------------------------ OS Timer Hook ------------------------
extern void OS_TIMER_TICK(void){
    TCP_UPDATE();
    NIC_PROCESS();
}
