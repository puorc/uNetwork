#include "IPController.h"

Result IPController::recv() const {
    auto[data, protocol, size] = eth.recv();
    if (protocol == static_cast<uint16_t>(Ethernet::protocol::ARP)) {
        arp.recv(data, size);
        return Result{.data=nullptr, .protocol=0, .size=0};
    } else if (protocol == static_cast<uint16_t>(Ethernet::protocol::IPv4)) {
        auto *ipv4 = reinterpret_cast<ipv4_t const *>(data);
        size_t header_length = (ipv4->ver_hl & 0x0f) * 4;
        return Result{.data=data + header_length, .protocol=ipv4->protocol, .size=
        ntohs(ipv4->datagram_len) - header_length};
    } else {
        std::cout << "Invalid IP packet. not supported" << std::endl;
        std::cout.flush();
        return Result{.data = nullptr, .protocol = 0, .size = 0};
    }
}

ssize_t IPController::send(uint32_t dst_ip, uint8_t protocol, uint8_t *data, size_t len) const {
    uint32_t gateway = rtable.lookup(dst_ip);
    if (gateway == RouteTable::end) {
        std::cout << "No route found." << std::endl;
        return 0;
    }
//    auto dmac = arp.query(gateway);
//    if (!dmac.has_value()) {
//        std::cout << "No ARP mac found." << std::endl;
//        std::cout.flush();
//        return 0;
//    }

    size_t buf_size = sizeof(struct ipv4_t) + len;
    auto *buf = new uint8_t[buf_size];
    uint8_t *ptr = buf;

    auto *ipv4 = reinterpret_cast<ipv4_t *>(ptr);
    ipv4->ver_hl = IPV4_VERSION | IPV4_HEADER_LENGTH;

    // no type of service
    ipv4->tos = 0;
    ipv4->datagram_len = htons(20 + len);

    // don't do fragmentation
    ipv4->id = 0;
    ipv4->flags_frag = 0x0040;

    // fix ttl
    ipv4->ttl = 0x40;

    ipv4->protocol = protocol;
    // calculate later
    ipv4->checksum = 0;
    ipv4->src_ip = device.ip_addr;
    ipv4->dst_ip = dst_ip;
    ipv4->checksum = checksum(ipv4, 20, 0);

    ptr += sizeof(struct ipv4_t);
    memcpy(ptr, data, len);
    ssize_t n = eth.send({0x86, 0x3b, 0x9a, 0x02, 0x85, 0x13}, Ethernet::protocol::IPv4, buf, buf_size);
    delete[] buf;
    return n;
}

uint16_t IPController::calculate_checksum(IPController::ipv4_t *ip) {
    uint16_t res = 0;

    uint16_t *ptr = reinterpret_cast<uint16_t *>(ip);

    for (int i = 0; i < 5; i++) {
        res += *ptr;
        ++ptr;
    }
    // skip checksum
    ++ptr;

    for (int i = 0; i < 4; i++) {
        res += *ptr;
        ++ptr;
    }
    res = ~res;
    return res;
}

IPController::IPController(const Ethernet &eth, ARPController &arp, const NetworkDevice &device,
                           const RouteTable &rtable) :
        eth(eth), arp(arp), device(device), rtable(rtable) {
    arp.send(rtable.lookup(0));
    recv();
}

