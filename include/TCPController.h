#ifndef TCPIP_TCPCONTROLLER_H
#define TCPIP_TCPCONTROLLER_H

#include "utils.h"
#include "IPController.h"
#include "TCPConnection.h"

class TCPController {
private:
    IPController &ip;
    TCPConnection conn;
public:
    TCPController(IPController &ip);

    Result recv();

    ssize_t send(int sockfd, uint8_t *data, size_t size);
};


#endif //TCPIP_TCPCONTROLLER_H
