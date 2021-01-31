//
// Created by pu on 1/13/21.
//

#ifndef TCPIP_UTILS_H
#define TCPIP_UTILS_H

#include <cstdint>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct Result {
    uint8_t const *data;
    uint16_t protocol;
    size_t size;
};

uint32_t ip_parse(char const *addr);

uint16_t checksum(void *addr, int count, int start_sum);

int tcp_udp_checksum(uint32_t saddr, uint32_t daddr, uint8_t proto,
                     uint8_t *data, uint16_t len);

#endif //TCPIP_UTILS_H
