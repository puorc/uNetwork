#include "TCPController.h"

void TCPController::recv() {
    auto[data, protocol, size] = ip.recv();
    if (size != 0) {
        auto *tcp = (tcp_t *) data;
        std::shared_lock lck(mutex_);
        int fd = port_fd_map[ntohs(tcp->dst_port)];
        if (connections.find(fd) != connections.end()) {
            auto conn = connections[fd];
            lck.unlock();
            conn->recv(data, size);
        }
    }
}

ssize_t TCPController::write(int fd, uint8_t const *data, size_t size) {
    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        lock.unlock();
        return conn->write(data, size);
    } else {
        return -1;
    }
}

int TCPController::alloc() {
    std::unique_lock lock(mutex_);
    uint16_t src_port = ++_port;
    int fd = ++_fd;
    port_fd_map[src_port] = fd;
    auto conn = std::make_shared<TCPConnection>(src_port, device.ip_addr, ip);
    connections[fd] = conn;
    return fd;
}

void TCPController::init(int fd, uint32_t dst_ip, uint16_t dst_port, std::function<void(int)> init_cb) {
    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        lock.unlock();
        conn->onEstablished(std::move(init_cb));
        conn->connect(dst_ip, dst_port);
    }
}

ssize_t TCPController::read(int fd, uint8_t *buf, size_t size) {
    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        lock.unlock();
        return conn->read(buf, size);
    }
    return 0;
}

int TCPController::close(int fd) {
    std::shared_lock lock(mutex_);
    if (connections.find(fd) != connections.end()) {
        auto conn = connections[fd];
        lock.unlock();
        conn->close();
        return 0;
    }
    return -1;
}

int TCPController::getname(int sockfd, struct sockaddr *addr, socklen_t *addrlen, bool is_peer) {
    std::shared_lock lock(mutex_);
    if (connections.find(sockfd) != connections.end()) {
        auto conn = connections[sockfd];
        lock.unlock();
        auto *ip_addr = reinterpret_cast<sockaddr_in *>(addr);
        ip_addr->sin_addr.s_addr = is_peer ? conn->_dst_ip : conn->_src_ip;
        ip_addr->sin_port = is_peer ? conn->_dst_port : conn->_src_port;
        ip_addr->sin_family = AF_INET;
        *addrlen = sizeof(sockaddr_in);
        return 0;
    }
    return -1;
}

int TCPController::poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    int total_polls = 0;
    while (true) {
        for (int i = 0; i < nfds; i++) {
            struct pollfd pfd = fds[i];
            std::shared_lock lock(mutex_);
            if (connections.find(pfd.fd) != connections.end()) {
//                std::cout << "query on fd " << pfd.fd << "for event " << pfd.events << std::endl;
                auto conn = connections[pfd.fd];
                lock.unlock();
//                short flags = 0;
//                if (conn->hasRead()) {
//                    flags |= (POLLRDNORM | POLLRDBAND | POLLIN | POLLPRI);
//                }
//                flags |= (POLLOUT | POLLWRNORM | POLLWRBAND);
                if ((conn->hasRead() & pfd.events) != 0) {
//                    std::cout << "return on fd " << pfd.fd << "for event " << conn->hasRead() << std::endl;
                    fds[i].revents = conn->hasRead();
                    total_polls++;
                }
            }
        }
        if (total_polls > 0) {
            break;
        }

    }
    return total_polls;
}
