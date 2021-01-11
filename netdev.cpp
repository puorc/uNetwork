#include "netdev.h"

static inline uint32_t ip_parse(char *addr) {
    uint32_t dst = 0;

    if (inet_pton(AF_INET, addr, &dst) != 1) {
        perror("ERR: Parsing inet address failed");
        exit(1);
    }

    return ntohl(dst);
}

static struct netdev *netdev_alloc(char *addr, char *hwaddr, uint32_t mtu) {
    struct netdev *dev = static_cast<netdev *>(malloc(sizeof(struct netdev)));

    dev->addr = ip_parse(addr);

    sscanf(hwaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dev->hwaddr[0],
           &dev->hwaddr[1],
           &dev->hwaddr[2],
           &dev->hwaddr[3],
           &dev->hwaddr[4],
           &dev->hwaddr[5]);

    dev->addr_len = 6;
    dev->mtu = mtu;

    return dev;
}

struct netdev *get_net_dev() {
    return netdev_alloc("10.0.0.4", "00:0c:29:6d:50:25", 1500);
}



