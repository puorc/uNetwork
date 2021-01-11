#ifndef TCPIP_ETHERNET_H
#define TCPIP_ETHERNET_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include "tuntap_if.h"

struct ethernet_t {
    uint8_t dmac[6];
    uint8_t smac[6];
    uint16_t ethertype;
//    uint8_t payload[];
} __attribute__((packed));

void send_ethernet(uint8_t *smac, uint8_t *dmac, uint16_t type, uint8_t *data, int len);
void send_ethernet_str(char *smac, char *dmac, uint16_t type, uint8_t *data, int len);

#endif //TCPIP_ETHERNET_H
