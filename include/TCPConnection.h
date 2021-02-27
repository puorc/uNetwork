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
#include <poll.h>
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
        LISTEN,
        SYN_SENT,
        SYN_RECEIVED,
        ESTABLISHED,
        FIN_WAIT_1,
        FIN_WAIT_2,
        CLOSE_WAIT,
        CLOSING,
        LAST_ACK,
        TIME_WAIT,
        CLOSED,
    };
    IPController const &ip_ctrl_;

    uint32_t seq_number_{0};
    uint32_t ack_number_{0};
    uint16_t src_port_;
    uint16_t dst_port_{};
    uint32_t src_ip_;
    uint32_t dst_ip_{};
    State state_{State::CLOSED};

    short poll_flags_;

    std::function<void(int)> established_cb_;
    std::function<void()> close_cb_;

    // to send
    std::mutex write_m_;
    uint32_t send_base_{0};
    std::deque<uint8_t> _send_buf;
    std::deque<uint8_t> _ack_buf;

    // to receive
    std::mutex _read_m;
    std::deque<uint8_t> _recv_buf;
    std::condition_variable _cv;
    bool _buffer_ready{false};

    ssize_t
    send(const uint8_t *data, size_t len, uint16_t flags, uint16_t src_port, uint16_t dst_port, uint32_t seq_number,
         uint32_t ack_number, uint32_t src_ip, uint32_t dst_ip);

    ssize_t send(const uint8_t *data, size_t len, uint16_t flags);

    void send();

    void inc_seq_number(uint16_t flags, size_t payload_size);

    void inc_ack_number(uint32_t inc);

    static uint16_t get_flags(std::initializer_list<uint16_t> masks, uint16_t header_len = sizeof(struct tcp_t));

    static bool has_ack(uint16_t flags) {
        return ((flags & ACK_MASK) != 0);
    }

    static bool has_syn(uint16_t flags) {
        return ((flags & SYN_MASK) != 0);
    }

    static bool has_fin(uint16_t flags) {
        return ((flags & FIN_MASK) != 0);
    }

    static bool has_rst(uint16_t flags) {
        return ((flags & RST_MASK) != 0);
    }

    static uint32_t nl_add_hl(uint32_t na, uint32_t hb) {
        return htonl(ntohl(na) + hb);
    }

public:
    TCPConnection(uint16_t src_port, uint32_t src_ip, const IPController &ip) : ip_ctrl_(ip),
                                                                                src_port_(src_port),
                                                                                src_ip_(src_ip) {}

    uint16_t get_dst_port() const {
        return dst_port_;
    }

    uint16_t get_src_port() const {
        return src_port_;
    }

    uint32_t get_src_ip() const {
        return src_ip_;
    }

    uint32_t get_dst_ip() const {
        return dst_ip_;
    }

    bool is_connected() const {
        return state_ == State::ESTABLISHED;
    }

    void recv(uint8_t const *data, size_t len, uint32_t from_ip) noexcept;

    void send(const uint8_t *data, size_t len);

    short hasRead() const { return poll_flags_; }

    // POSIX APIs
    void connect(uint32_t dst_ip, uint16_t dst_port);

    ssize_t read(uint8_t *buf, size_t size);

    ssize_t write(uint8_t const *buf, size_t size);

    void close();

    void on_established(std::function<void(int)> cb) {
        established_cb_ = std::move(cb);
    }

    void on_close(std::function<void()> cb) {
        close_cb_ = std::move(cb);
    }
};


#endif //TCPIP_TCPCONNECTION_H
