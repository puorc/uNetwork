//
// Created by pu on 1/9/21.
//

#ifndef TCPIP_IPV4_H
#define TCPIP_IPV4_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "ethernet.h"
#include "constants.h"
#include <iostream>

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

void ipv4_send(uint32_t src_ip, uint32_t dst_ip, uint8_t protocol, uint8_t *data, int len);

void ipv4_receive(uint8_t *data, int len, uint8_t **ptr, int *out_len);

#endif //TCPIP_IPV4_H
