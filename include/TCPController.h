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
    int _port{49152};
    IPController const &ip;
    NetworkDevice const &device;
    std::unordered_map<uint16_t, std::shared_ptr<TCPConnection>> connections;
public:
    explicit TCPController(const IPController &ip, const NetworkDevice &device);

    uint16_t alloc();

    void recv();

    void send(uint16_t port, const uint8_t *data, size_t size);

    void init(uint16_t port, uint32_t dst_ip, uint16_t dst_port);
};


#endif //TCPIP_TCPCONTROLLER_H
