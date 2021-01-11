#ifndef TCPIP_IPC_H
#define TCPIP_IPC_H

#include <unistd.h>
#include <sys/types.h>
#include <cstdint>
#include <sys/un.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <ftw.h>

struct socket_t {
    pid_t pid;
    int domain;
    int type;
    int protocol;
};

#define IPC_SOCKET 0x0001
#define IPC_CONNECT 0x0002

struct ipc_msg {
    uint16_t type;
    pid_t pid;
    uint8_t data[];
} __attribute__((packed));

struct ipc_socket {
    int domain;
    int type;
    int protocol;
} __attribute__((packed));

struct ipc_err {
    int rc;
    int err;
    uint8_t data[];
} __attribute__((packed));

struct ipc_connect {
    int sockfd;
    struct sockaddr addr;
    socklen_t addrlen;
} __attribute__((packed));

void rx_loop();

#endif //TCPIP_IPC_H
