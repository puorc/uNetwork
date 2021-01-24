#ifndef TCPIP_ETHERNET_H
#define TCPIP_ETHERNET_H

#include <array>
#include "NetworkDevice.h"

class Ethernet {
private:
    const NetworkDevice &device;
    uint8_t *_buf;

    struct ethernet_t {
        uint8_t dmac[6];
        uint8_t smac[6];
        uint16_t ethertype;
    } __attribute__((packed));
public:
    explicit Ethernet(const NetworkDevice &device);

    virtual ~Ethernet();

    Ethernet(Ethernet const &) = delete;

    void operator=(Ethernet const &) = delete;

    enum class protocol : uint16_t {
        IPv4 = 0x0800,
        IPv6 = 0x86DD,
        ARP = 0x0806
    };

    using HwAddr = std::array<uint8_t, 6>;

    ssize_t send(const HwAddr &dmac, Ethernet::protocol type, uint8_t *data, size_t len);

    Result recv();
};


#endif
