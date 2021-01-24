#ifndef TCPIP_ARPCONTROLLER_H
#define TCPIP_ARPCONTROLLER_H

#include <cstdint>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <queue>
#include <iostream>
#include <unordered_map>
#include <optional>
#include "Ethernet.h"
#include "NetworkDevice.h"

class ARPController {
private:
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

    enum class Opcode : uint16_t {
        Request = 0x0100,
        Reply = 0x0200
    };

    using AddrTTL = std::pair<time_t, uint32_t>;

    const Ethernet &eth;
    const NetworkDevice &dev;
    std::unordered_map<uint32_t, std::pair<Ethernet::HwAddr, time_t>> table;
    std::priority_queue<AddrTTL, std::vector<AddrTTL>, std::greater<>> q;

    void reply(uint8_t const *data, size_t size) const;

public:
    ARPController(const Ethernet &eth, const NetworkDevice &dev);

    ARPController(ARPController const &) = delete;

    void operator=(ARPController const &) = delete;

    void recv(uint8_t const *data, size_t size);

    void send(uint32_t ip_addr) const;

    std::optional<Ethernet::HwAddr> query(uint32_t ip_addr);
};


#endif //TCPIP_ARPCONTROLLER_H
