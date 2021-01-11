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

void ipv4_send(uint32_t src_ip, uint32_t dst_ip, uint8_t protocol, uint8_t *data, int len) {
    struct ipv4_t ipv4;
    ipv4.ver_hl = IPV4_VERSION | IPV4_HEADER_LENGTH;
    // no type of service
    ipv4.tos = 0;
    ipv4.datagram_len = htobe16(20 + len);
    // don't do fragmentation
    ipv4.id = 0;
    ipv4.flags_frag = 0x0040;
    // fix timeline
    ipv4.ttl = 0x40;

    ipv4.protocol = protocol;
    // calculate later
    ipv4.checksum = 0;
    ipv4.src_ip = src_ip;
    ipv4.dst_ip = dst_ip;

    ipv4.checksum = calculate_checksum(&ipv4);

    uint8_t *packet = static_cast<uint8_t *>(malloc(sizeof(struct ipv4_t) + len));
    uint8_t *ptr = packet;
    memcpy(ptr, &ipv4, sizeof(struct ipv4_t));
    ptr += sizeof(struct ipv4_t);
    memcpy(ptr, data, len);
    send_ethernet_str("00:0c:29:6d:50:25", "aa:2e:9d:e9:38:ec", ETH_IPV4, packet, sizeof(struct ipv4_t) + len);
}