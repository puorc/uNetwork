#ifndef TCPIP_TCPCONTROLLER_H
#define TCPIP_TCPCONTROLLER_H

#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include "utils.h"
#include "IPController.h"
#include "TCPConnection.h"

class TCPController {
private:
    mutable std::shared_mutex mutex_;
    int _port{49156};
    int _fd{1000};
    IPController const &ip;
    NetworkDevice const &device;
    std::unordered_map<int, std::shared_ptr<TCPConnection>> connections;
    std::unordered_map<uint16_t, int> port_fd_map;
public:
    explicit TCPController(const IPController &ip, const NetworkDevice &device) : ip(ip), device(device),
                                                                                  connections(), port_fd_map() {};

    int alloc();

    void recv();

    void send(int fd, uint8_t const *data, size_t size, std::function<void(int)> callback);

    void init(int fd, uint32_t dst_ip, uint16_t dst_port, std::function<void(int)> init_cb);

    ssize_t read(int fd, uint8_t *buf, size_t size);
};


#endif //TCPIP_TCPCONTROLLER_H
