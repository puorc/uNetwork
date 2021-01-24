#include "NetworkDevice.h"

NetworkDevice::NetworkDevice() : mtu(1500), ip_addr(ip_parse("10.0.0.4")) {
    _tun_id = tun_init();
    mac_addr = new uint8_t[6];
    sscanf("00:0c:29:6d:50:25", "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac_addr[0],
           &mac_addr[1],
           &mac_addr[2],
           &mac_addr[3],
           &mac_addr[4],
           &mac_addr[5]);
}

int NetworkDevice::tun_init() {
    struct ifreq ifr;
    int fd, err;
    if ((fd = open("/dev/net/tap", O_RDWR)) < 0) {
        perror("Cannot open TUN/TAP device.");
        exit(1);
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }
    if ((err = ioctl(fd, TUNSETIFF, &ifr)) < 0) {
        close(fd);
        throw std::system_error(EBADF, std::system_category(), "Unable to open tap device");
    }
    return fd;
}

ssize_t NetworkDevice::tun_read(uint8_t *buf, std::size_t len) const {
    return read(_tun_id, buf, len);
}

ssize_t NetworkDevice::tun_write(uint8_t *buf, std::size_t len) const {
    return write(_tun_id, buf, len);
}

NetworkDevice::~NetworkDevice() {
    delete[] mac_addr;
    close(_tun_id);
}
