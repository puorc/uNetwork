#include <cstdint>
#include <cstring>
#include <iostream>
#include "ethernet.h"
#include "constants.h"
#include "network.h"
#include "ipv4.h"
#include "arp.h"
#include "TCPConnection.h"

const int MAX_PACKET_SIZE = 8192;

static void send_test() {
    uint8_t *start = static_cast<uint8_t *>(malloc(sizeof(tcp_t)));
    uint8_t *payload = start;

//    struct ipv4_t ipv4;
//    ipv4.ver_hl = 0x45;
//    ipv4.tos = 0;
//    ipv4.datagram_len = 0x2800;
//    ipv4.id = 0;
//    ipv4.flags_frag = 0x0040;
//    ipv4.ttl = 0x40;
//    ipv4.protocol = 0x06;
//    ipv4.checksum = 0x4397;
//    ipv4.src_ip = 0x0400000a;
//    ipv4.dst_ip = 0x4ec13ad8;

    struct tcp_t tcp;
    tcp.src_port = 0xebab;
    tcp.dst_port = 0x5000;
    tcp.seq = 0;
    tcp.ack = 0;
    tcp.flags = 0x0250;
    tcp.window = 0xbdad;
    tcp.checksum = 0x5cb2;
    tcp.urgent_pt = 0;
    memcpy(payload, &tcp, sizeof(tcp_t));
    ipv4_send(0x0400000a, 0x4ec13ad8, 0x06, start, sizeof(struct tcp_t));


//    struct tcp_t *packet = new tcp_t();
//    packet->destmac=0x0c00ec38e99d2eaa;
//    packet->srcmac=0x25506d29;
//    packet->type=0x0008;
//    packet->ip_flags=0x0045;
//    packet->datagram=0x2800;
//    packet->id=0;
//    packet->frag=0;
//    packet->protocol_flag=0x0640;
//    packet->ip_checksum=0x4397;
//    packet->src_ip=0x0400000a;
//    packet->dest_ip = 0x4ec13ad8;
//    packet->source = 0xebab;
//    packet->frag=0x0040;
//    packet->dest = 0x5000;
//    packet->seq = 0;
//    packet->ack = 0;
//    packet->flags=0x0250;
//    packet->window=0xbdad;
//    packet->checksum=0x5cb2;
//    packet->urgent=0;

}

void read_loop() {
    tun_init();
    uint8_t buf[MAX_PACKET_SIZE];
    TCPConnection s(0x2ed93ad8, 80);
    s.init();
    bool doit = false;
    int n;
    while ((n = tun_read(buf, MAX_PACKET_SIZE)) > 0) {
        uint8_t *ptr;
        int len;
        switch (receive_ethernet(buf, n, &ptr, &len)) {
            case ETH_IPV4: {
                std::cout << "ipv4 packet" << std::endl;
                std::cout.flush();
                uint8_t *wptr;
                int wlen;
                ipv4_receive(ptr, len, &wptr, &wlen);
                s.receive(wptr, wlen);
                const char *tosend = "GET / HTTP/1.1\r\nHost: google.com:80\r\nConnection: close\r\n\r\n";
                if (!doit) {
                    s.send((uint8_t *) tosend, 58);
                    doit = true;
                }
                break;
            }
            case ETH_ARP:
                std::cout << "arp packet" << std::endl;
                std::cout.flush();
                process_arp(ptr);
                break;
            case ETH_IPV6:
                std::cout << "ipv6 packet" << std::endl;
                std::cout.flush();
                break;
            default:
                std::cout << "protocol not supported" << std::endl;
                std::cout.flush();
        }
    }
}