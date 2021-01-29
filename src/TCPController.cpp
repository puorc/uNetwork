#include "TCPController.h"

void TCPController::recv() {
//    std::shared_lock lock(mutex_);
    auto[data, protocol, size] = ip.recv();
    if (size != 0) {
        auto *tcp = (tcp_t *) data;
        int fd = port_fd_map[ntohs(tcp->dst_port)];
        if (connections.find(fd) != connections.end()) {
            auto conn = connections[fd];
            conn->recv(data, size);
        }
    }
}

void TCPController::send(int fd, const uint8_t *data, size_t size) {
//    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        conn->send(data, size);
    }
}

int TCPController::alloc() {
//    std::unique_lock lock(mutex_);
    uint16_t src_port = ++_port;
    int fd = ++_fd;
    port_fd_map[src_port] = fd;
    auto conn = std::make_shared<TCPConnection>(src_port, device.ip_addr, ip);
    conn->onEstablished([]() {
        std::cout << "established!" << std::endl;
        std::cout.flush();
    });
    connections[fd] = conn;
    return fd;
}

void TCPController::init(int fd, uint32_t dst_ip, uint16_t dst_port) {
//    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        conn->init(dst_ip, dst_port);
    }
}
