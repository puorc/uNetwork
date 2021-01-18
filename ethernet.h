#ifndef TCPIP_ETHERNET_H
#define TCPIP_ETHERNET_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <arpa/inet.h>
#include "tuntap_if.h"

struct ethernet_t {
    uint8_t dmac[6];
    uint8_t smac[6];
    uint16_t ethertype;
} __attribute__((packed));

enum class ethernet_protocol : uint16_t {
    IPv4 = 0x0800,
    IPv6 = 0x86DD,
    ARP = 0x0806
};

ssize_t ethernet_send(uint8_t *smac, uint8_t *dmac, ethernet_protocol type, uint8_t *data, size_t len);

ssize_t ethernet_send_with_mac(char const *smac, char const *dmac, ethernet_protocol type, uint8_t *data, size_t len);

ssize_t ethernet_recv(uint8_t *buf, ethernet_protocol *protocol);

#endif //TCPIP_ETHERNET_H
