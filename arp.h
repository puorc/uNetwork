#ifndef TCPIP_ARP_H
#define TCPIP_ARP_H

#include <cstdint>
#include <cstring>
#include "netdev.h"
#include "ethernet.h"
#include "constants.h"
#include "tuntap_if.h"

struct arp_t {
    uint16_t hw_type;
    uint16_t protocol_type;
    uint8_t hw_addr_len;
    uint8_t protocol_addr_len;
    uint16_t opcode;
    uint8_t sender_hw_addr[6];
    uint32_t sender_protocol_addr;
    uint8_t target_hw_addr[6];
    uint32_t target_protocol_addr;
}  __attribute__((packed));

void process_arp(uint8_t *data);

#endif //TCPIP_ARP_H
