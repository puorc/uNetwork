#ifndef TCPIP_NETDEV_H
#define TCPIP_NETDEV_H

#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>

struct netdev *get_net_dev();

struct netdev {
    uint32_t addr;
    uint8_t addr_len;
    uint8_t hwaddr[6];
    uint32_t mtu;
};

#endif //TCPIP_NETDEV_H
