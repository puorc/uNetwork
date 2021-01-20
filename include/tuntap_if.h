#ifndef TCPIP_TUNTAP_IF_H
#define TCPIP_TUNTAP_IF_H

#include <sys/ioctl.h>
#include <net/if.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <cstdint>

void tun_init();

ssize_t tun_read(uint8_t *buf, size_t len);

ssize_t tun_write(uint8_t *buf, size_t len);

#endif //TCPIP_TUNTAP_IF_H
