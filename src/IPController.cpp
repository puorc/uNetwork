#include "IPController.h"

Result IPController::recv() {
    auto[data, protocol, size] = eth.recv();
    if (protocol == static_cast<uint16_t>(Ethernet::protocol::ARP)) {
        arp.recv(data, size);
        return Result{.data=nullptr, .protocol=0, .size=0};
    }
    if (size < sizeof(struct ipv4_t)) {
        std::cout << "Invalid IPv4 packet. size too small" << std::endl;
        std::cout.flush();
        return Result{.data = nullptr, .protocol = 0, .size = 0};
    }

    auto *ipv4 = reinterpret_cast<ipv4_t *>(data);
//    if (ipv4->protocol != static_cast<uint8_t >(protocol::TCP)) {
//        std::cout << "Not a TCP packet!";
//        std::cout.flush();
//        return Result{.data = nullptr, .protocol = ipv4->protocol, .size = 0};
//    }
    size_t header_length = (ipv4->ver_hl & 0x0f) * 4;
    return Result{.data=data + header_length, .protocol=static_cast<uint8_t >(protocol::TCP), .size=
    ntohs(ipv4->datagram_len) - header_length};
}

ssize_t IPController::send(uint32_t dst_ip, uint8_t protocol, uint8_t *data, size_t len) {
    uint32_t gateway = rtable.lookup(dst_ip);
    if (gateway == RouteTable::end) {
        std::cout << "No route found." << std::endl;
        return 0;
    }

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
    ipv4->checksum = calculate_checksum(ipv4);

    ptr += sizeof(struct ipv4_t);
    memcpy(ptr, data, len);
    ssize_t n = eth.send(arp.query(gateway), Ethernet::protocol::IPv4, buf, buf_size);
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

IPController::IPController(Ethernet &eth, ARPController &arp, NetworkDevice &device, RouteTable &rtable) : eth(eth),
                                                                                                           arp(arp),
                                                                                                           device(device),
                                                                                                           rtable(rtable) {
    arp.send(rtable.lookup(0));
    recv();
}

