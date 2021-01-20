#include "tuntap_if.h"

static int tun_id;
static char const *dev = "tap0";

void tun_init() {
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
        perror("Err: ioctl error");
        close(fd);
        exit(1);
    }
    tun_id = fd;
}

ssize_t tun_read(uint8_t *buf, size_t len) {
    return read(tun_id, buf, len);
}

ssize_t tun_write(uint8_t *buf, size_t len) {
    return write(tun_id, buf, len);
}

