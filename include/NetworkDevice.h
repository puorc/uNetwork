#ifndef TCPIP_NETWORKDEVICE_H
#define TCPIP_NETWORKDEVICE_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <system_error>
#include "utils.h"

class NetworkDevice {
private:
    int _tun_id;

    int tun_init();

    char const *dev = "tap0";
public:
    const uint32_t mtu;
    const uint32_t ip_addr;
    const uint8_t *mac_addr;

    NetworkDevice();

    virtual ~NetworkDevice();

    NetworkDevice(NetworkDevice const &) = delete;

    void operator=(NetworkDevice const &) = delete;

    ssize_t tun_read(uint8_t *buf, std::size_t len) const;

    ssize_t tun_write(uint8_t *buf, std::size_t len) const;
};


#endif //TCPIP_NETWORKDEVICE_H
