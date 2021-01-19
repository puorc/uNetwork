#include "ipv4.h"

const uint8_t IPV4_VERSION = 0x40;
// don't support option so far.
const uint8_t IPV4_HEADER_LENGTH = 0x05;

static uint16_t calculate_checksum(struct ipv4_t *ip) {
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

ssize_t ipv4_send(uint32_t src_ip, uint32_t dst_ip, uint8_t protocol, uint8_t *data, size_t len) {
    size_t buf_size = sizeof(struct ipv4_t) + len;
    auto *buf = new uint8_t[buf_size];
    uint8_t *ptr = buf;

    struct ipv4_t *ipv4 = reinterpret_cast<ipv4_t *>(ptr);
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
    ipv4->src_ip = src_ip;
    ipv4->dst_ip = dst_ip;
    ipv4->checksum = calculate_checksum(ipv4);

    ptr += sizeof(struct ipv4_t);
    memcpy(ptr, data, len);
    ssize_t n = ethernet_send_with_mac("00:0c:29:6d:50:25", "06:fb:91:e6:e0:4e", ethernet_protocol::IPv4, buf,
                                       sizeof(struct ipv4_t) + len);
    delete[] buf;
    return n;
}

Result ipv4_recv(uint8_t *data, size_t len) {
    if (len < sizeof(struct ipv4_t)) {
        std::cout << "Invalid IPv4 packet. size too small" << std::endl;
        std::cout.flush();
        return Result{.data = nullptr, .protocol = 0, .size = 0};
    }

    auto *ipv4 = reinterpret_cast<ipv4_t *>(data);
    if (ipv4->protocol != IP_TCP) {
        std::cout << "Not a TCP packet!";
        std::cout.flush();
        return Result{.data = nullptr, .protocol = ipv4->protocol, .size = 0};
    }

    size_t header_length = (ipv4->ver_hl & 0x0f) * 4;
    return Result{.data=data + header_length, .protocol=IP_TCP, .size=ntohs(ipv4->datagram_len) - header_length};
}