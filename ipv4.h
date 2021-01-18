#ifndef TCPIP_IPV4_H
#define TCPIP_IPV4_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "ethernet.h"
#include "utils.h"

const uint8_t IP_TCP = 0x06;

struct ipv4_t {
    uint8_t ver_hl;
    uint8_t tos;
    uint16_t datagram_len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
} __attribute__((packed));

ssize_t ipv4_send(uint32_t src_ip, uint32_t dst_ip, uint8_t protocol, uint8_t *data, size_t len);

Result ipv4_recv(uint8_t *data, size_t len);

#endif //TCPIP_IPV4_H
