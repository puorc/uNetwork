#ifndef TCPIP_IPCONTROLLER_H
#define TCPIP_IPCONTROLLER_H

#include <cstdint>
#include "NetworkDevice.h"
#include "ARPController.h"
#include "Ethernet.h"
#include "RouteTable.h"
#include "utils.h"

class IPController {
private:
    Ethernet &eth;
    ARPController &arp;
    NetworkDevice &device;
    RouteTable &rtable;

    const uint8_t IPV4_VERSION = 0x40;
    const uint8_t IPV4_HEADER_LENGTH = 0x05;

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

    enum class protocol : uint8_t {
        TCP = 0x06
    };

    uint16_t calculate_checksum(struct ipv4_t *ip);

public:
    IPController(Ethernet &eth, ARPController &arp, NetworkDevice &device, RouteTable &rtable);

    ssize_t send(uint32_t dst_ip, uint8_t protocol, uint8_t *data, size_t len);

    Result recv();
};


#endif //TCPIP_IPCONTROLLER_H
