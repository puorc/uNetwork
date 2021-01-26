#ifndef TCPIP_TCPCONNECTION_H
#define TCPIP_TCPCONNECTION_H

#include <queue>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <functional>
#include <utility>
#include "utils.h"
#include "IPController.h"

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

const uint8_t IP_TCP = 0x06;

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

    std::function<void()> _established_cb;
    std::function<void()> _close_cb;
    std::function<void(int)> _send_cb;
    std::function<void(uint8_t *, size_t)> _recv_cb;

    std::queue<struct tcp_t *> q;
    uint32_t seq_number;
    uint32_t ack_number;
    uint32_t send_base;
    State state;
    uint16_t _src_port;
    uint16_t _dst_port;
    uint32_t _src_ip;
    uint32_t _dst_ip;
    IPController const &ip;

    ssize_t send(const uint8_t *data, size_t len, uint16_t flags);

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
    TCPConnection(uint16_t port, uint32_t src_ip, IPController const &ip)
            : q(), state(State::CLOSED), _src_port(htons(port)), _src_ip(src_ip), _dst_port(0), _dst_ip(0),
              ack_number(0), send_base(0), ip(ip) {
        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_int_distribution<uint32_t> uniform_dist;
        seq_number = uniform_dist(e1);
    }

    void recv(uint8_t const *data, size_t len);

    void send(const uint8_t *data, size_t len);

    void init(uint32_t dst_ip, uint16_t dst_port);

    void onEstablished(std::function<void()> cb) {
        _established_cb = std::move(cb);
    }

    void onClose(std::function<void()> cb) {
        _close_cb = std::move(cb);
    }

    void onSend(std::function<void(int)> cb) {
        _send_cb = std::move(cb);
    }

    void onRecv(std::function<void(uint8_t *, size_t)> cb) {
        _recv_cb = std::move(cb);
    }
};


#endif //TCPIP_TCPCONNECTION_H
