#ifndef TCPIP_TCP_H
#define TCPIP_TCP_H

struct tcp_t {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint16_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_pt;
//    uint8_t data[];
} __attribute__((packed));


#endif //TCPIP_TCP_H
