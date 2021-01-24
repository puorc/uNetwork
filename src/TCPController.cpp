#include "TCPController.h"

Result TCPController::recv() {
    auto[data, protocol, size] = ip.recv();
    if (size != 0) {
        conn.receive(data, size);
    }
    return Result{.data=nullptr, .protocol=0, .size = 0,};
}

ssize_t TCPController::send(int sockfd, uint8_t *data, size_t size) {
    conn.send(data, size);
}

TCPController::TCPController(IPController &ip) : ip(ip), conn(0x2ed93ad8, 80, ip) {
    conn.init();
}
