#ifndef TCPIP_TCPCONNECTION_H
#define TCPIP_TCPCONNECTION_H

#include "utils.h"
#include <queue>
#include <cstdint>
#include <cstdlib>

struct tcp_t {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint16_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_pt;
} __attribute__((packed));

const uint16_t DEFAULT_MASK = 0x0000;
const uint16_t FIN_MASK = 0x0100;
const uint16_t SYN_MASK = 0x0200;
const uint16_t RST_MASK = 0x0400;
const uint16_t PSH_MASK = 0x0800;
const uint16_t ACK_MASK = 0x1000;
const uint16_t URG_MASK = 0x2000;


enum class State {
    CLOSED,
    SYN_SEND,
    ESTABLISHED,
    FIN_WAIT_1,
    FIN_WAIT_2,
    TIME_WAIT
};

class TCPConnection {
private:
    std::queue<struct tcp_t *> q;
    uint32_t seq_number;
    uint32_t ack_number;
    State state;
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t dst_ip;
    uint32_t src_ip;

    void send(uint8_t *data, size_t len, uint16_t flags);
public:
    TCPConnection(uint32_t dst_ip, uint16_t port)
            : q(), state(State::CLOSED), dst_port(htons(port)), src_port(0xebac), dst_ip(dst_ip), src_ip(0x0400000a), ack_number(0) {
        seq_number = rand() % 100000;
    }

    void receive(uint8_t *data, size_t len);

    void send(uint8_t *data, size_t len);

    void init();
};


#endif //TCPIP_TCPCONNECTION_H
