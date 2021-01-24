#include "utils.h"

uint32_t ip_parse(char const *addr) {
    uint32_t dst = 0;
    if (inet_pton(AF_INET, addr, &dst) != 1) {
        return dst;
    }
    return dst;
}

static uint32_t sum_every_16bits(void *addr, int count) {
    uint32_t sum = 0;
    uint16_t *ptr = static_cast<uint16_t *>(addr);

    while (count > 1) {
        /*  This is the inner loop */
        sum += *ptr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if (count > 0)
        sum += *(uint8_t *) ptr;

    return sum;
}

static uint16_t checksum(void *addr, int count, int start_sum) {
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     * Taken from https://tools.ietf.org/html/rfc1071
     */
    uint32_t sum = start_sum;

    sum += sum_every_16bits(addr, count);

    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

int tcp_udp_checksum(uint32_t saddr, uint32_t daddr, uint8_t proto,
                     uint8_t *data, uint16_t len) {
    uint32_t sum = 0;

    sum += saddr;
    sum += daddr;
    sum += htons(proto);
    sum += htons(len);

    return checksum(data, len, sum);
}

//static uint16_t calculate_checksum(uint8_t *tcp) {
//    uint16_t res = 0;
//    struct pseudo_ip pseudo;
//    pseudo.dst_ip = 0x4ec13ad8;
//    pseudo.src_ip = 0x0400000a;
//    pseudo.reserved = 0;
//    pseudo.protocol = IP_TCP;
//    pseudo.length = htobe16(20);
//
//    uint8_t *ptr = static_cast<uint8_t *>(malloc(sizeof(struct pseudo_ip) + sizeof(struct tcp_t)));
//    uint8_t *head = ptr;
//    memcpy(ptr, &pseudo, sizeof(struct pseudo_ip));
//    ptr += sizeof(struct pseudo_ip);
//    memcpy(ptr, tcp, sizeof(struct tcp_t));
//    int len = sizeof(struct pseudo_ip) + sizeof(struct tcp_t);
//
//    uint16_t *p = reinterpret_cast<uint16_t *>(head);
//
//    for (int i = 0; i < len / 2; i++) {
//        res += *p;
//        ++p;
//    }
//    res = ~res;
//    return res;
//}