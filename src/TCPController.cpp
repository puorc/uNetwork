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

ssize_t TCPController::write(int fd, uint8_t const *data, size_t size) {
//    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        return conn->write(data, size);
    } else {
        return -1;
    }
}

int TCPController::alloc() {
//    std::unique_lock lock(mutex_);
    uint16_t src_port = ++_port;
    int fd = ++_fd;
    port_fd_map[src_port] = fd;
    auto conn = std::make_shared<TCPConnection>(src_port, device.ip_addr, ip);
    connections[fd] = conn;
    return fd;
}

void TCPController::init(int fd, uint32_t dst_ip, uint16_t dst_port, std::function<void(int)> init_cb) {
//    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        conn->onEstablished(std::move(init_cb));
        conn->connect(dst_ip, dst_port);
    }
}

ssize_t TCPController::read(int fd, uint8_t *buf, size_t size) {
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        return conn->read(buf, size);
    }
    return 0;
}

int TCPController::close(int fd) {
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        conn->close();
        return 0;
    }
    return -1;
}
