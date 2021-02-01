#ifndef TCPIP_TCPCONNECTION_H
#define TCPIP_TCPCONNECTION_H

#include <queue>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <atomic>
#include <functional>
#include <utility>
#include <condition_variable>
#include <mutex>
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
    const size_t MSS = 1000;

    enum class State {
        CLOSED,
        SYN_SEND,
        ESTABLISHED,
        FIN_WAIT_1,
        FIN_WAIT_2,
        TIME_WAIT
    };
    IPController const &ip;

    std::function<void(int)> _established_cb;
    std::function<void()> _close_cb;

    uint32_t seq_number{0};
    uint32_t ack_number{0};

    State state{State::CLOSED};
    uint16_t _src_port;
    uint16_t _dst_port{};
    uint32_t _src_ip;
    uint32_t _dst_ip{};

    // to send
    std::mutex _write_m;
    uint32_t _send_base{0};
    std::deque<uint8_t> _send_buf;
    std::deque<uint8_t> _ack_buf;

    // to receive
    std::mutex _read_m;
    std::deque<uint8_t> _recv_buf;
    std::condition_variable _cv;
    bool _buffer_ready{false};

    ssize_t send(const uint8_t *data, size_t len, uint16_t flags);

    void send();

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
    TCPConnection(uint16_t src_port, uint32_t src_ip, const IPController &ip) : ip(ip), _src_port(htons(src_port)),
                                                                                _src_ip(src_ip) {}

    void recv(uint8_t const *data, size_t len) noexcept;

    void send(const uint8_t *data, size_t len);

    // POSIX APIs
    void connect(uint32_t dst_ip, uint16_t dst_port);

    ssize_t read(uint8_t *buf, size_t size);

    ssize_t write(uint8_t const *buf, size_t size);

    void onEstablished(std::function<void(int)> cb) {
        _established_cb = std::move(cb);
    }

    void onClose(std::function<void()> cb) {
        _close_cb = std::move(cb);
    }
};


#endif //TCPIP_TCPCONNECTION_H
