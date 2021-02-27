#ifndef TCPIP_TCPCONTROLLER_H
#define TCPIP_TCPCONTROLLER_H

#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include "utils.h"
#include "TCPConnection.h"

class TCPController {
private:
    std::shared_mutex mutex_;
    int next_port_{38360};
    int next_fd_{1500};

    IPController const &ip_ctrl;
    NetworkDevice const &device;

    std::unordered_map<int, std::shared_ptr<TCPConnection>> connections;
    std::unordered_map<uint16_t, int> port_fd_map;

    TCPConnection dummy_connection;
public:
    explicit TCPController(const IPController &ip, const NetworkDevice &device) : ip_ctrl(ip), device(device),
                                                                                  dummy_connection(next_port_ - 1,
                                                                                                   device.ip_addr,
                                                                                                   ip) {};

    int socket();

    void recv();

    void connect(int fd, uint32_t dst_ip, uint16_t dst_port, std::function<void(int)> init_cb);

    ssize_t write(int fd, uint8_t const *data, size_t size);

    ssize_t read(int fd, uint8_t *buf, size_t size);

    int poll(struct pollfd fds[], nfds_t nfds, int timeout);

    int close(int fd);

    int get_name(int sockfd, struct sockaddr *addr, socklen_t *addrlen, bool is_peer);
};


#endif //TCPIP_TCPCONTROLLER_H
