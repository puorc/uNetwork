#include "TCPController.h"

void TCPController::recv() {
    std::shared_lock lock(mutex_);
    auto[data, protocol, size] = ip.recv();
    if (size != 0) {
        auto *tcp = (tcp_t *) data;
        if (connections.find(ntohs(tcp->dst_port)) != connections.end()) {
            auto conn = connections[ntohs(tcp->dst_port)];
            conn->recv(data, size);
        }
    }
}

void TCPController::send(uint16_t port, const uint8_t *data, size_t size) {
    std::shared_lock lock(mutex_);
    if (connections.find(port) != connections.end()) {
        auto conn = connections[port];
        conn->send(data, size);
    }
}

TCPController::TCPController(const IPController &ip, const NetworkDevice &device) : ip(ip), device(device),
                                                                                    connections() {}

uint16_t TCPController::alloc() {
    std::unique_lock lock(mutex_);
    uint16_t dst_port = ++_port;
    auto conn = std::make_shared<TCPConnection>(dst_port, device.ip_addr, ip);
    conn->onEstablished([]() {
        std::cout << "established!" << std::endl;
        std::cout.flush();
    });
    connections[dst_port] = conn;
    return dst_port;
}

void TCPController::init(uint16_t port, uint32_t dst_ip, uint16_t dst_port) {
    std::shared_lock lock(mutex_);
    if (connections.find(port) != connections.end()) {
        auto conn = connections[port];
        conn->init(dst_ip, dst_port);
    }
}
