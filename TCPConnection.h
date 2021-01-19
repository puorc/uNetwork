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


class TCPConnection {
private:
    enum class State {
        CLOSED,
        SYN_SEND,
        ESTABLISHED,
        FIN_WAIT_1,
        FIN_WAIT_2,
        TIME_WAIT
    };

    std::queue<struct tcp_t *> q;
    uint32_t seq_number;
    uint32_t ack_number;
    uint32_t send_base;
    State state;
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t dst_ip;
    uint32_t src_ip;

    void send(uint8_t *data, size_t len, uint16_t flags);

    void inc_seq_number(uint16_t flags, size_t payload_size);

    void inc_ack_number(uint16_t flags, size_t payload_size);

    static uint16_t get_flags(std::initializer_list<uint16_t> masks, uint16_t header_len = sizeof(struct tcp_t));

    static bool is_ack(uint16_t flags) {
        return ((flags & ACK_MASK) != 0);
    }

    static bool is_syn(uint16_t flags) {
        return ((flags & SYN_MASK) != 0);
    }

    static bool is_fin(uint16_t flags) {
        return ((flags & FIN_MASK) != 0);
    }

    static uint32_t nl_add_hl(uint32_t na, uint32_t hb) {
        return htonl(ntohl(na) + hb);
    }

public:
    TCPConnection(uint32_t dst_ip, uint16_t port)
            : q(), state(State::CLOSED), dst_port(htons(port)), src_port(0xebab), dst_ip(dst_ip), src_ip(0x0400000a),
              ack_number(0), send_base(0) {
        seq_number = rand() % 100000;
    }

    void receive(uint8_t *data, size_t len);

    void send(uint8_t *data, size_t len);

    void init();
};


#endif //TCPIP_TCPCONNECTION_H
