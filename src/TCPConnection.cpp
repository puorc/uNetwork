#include <thread>
#include "TCPConnection.h"

ssize_t TCPConnection::send(const uint8_t *data, size_t len, uint16_t flags, uint16_t src_port, uint16_t dst_port,
                            uint32_t seq_number, uint32_t ack_number, uint32_t src_ip, uint32_t dst_ip) {
    size_t total_len = len + sizeof(struct tcp_t);
    auto *buf = new uint8_t[total_len];

    auto *tcp = reinterpret_cast<tcp_t *>(buf);
    tcp->src_port = htons(src_port);
    tcp->dst_port = htons(dst_port);
    tcp->seq = seq_number;
    tcp->ack = ack_number;
    tcp->flags = flags;
    tcp->window = 0xbdad;
    tcp->checksum = 0;
    tcp->urgent_pt = 0;

    std::copy(data, data + len, buf + sizeof(struct tcp_t));
    tcp->checksum = tcp_udp_checksum(src_ip, dst_ip, IP_TCP, buf, total_len);

    ssize_t n = ip_ctrl_.send(dst_ip, IP_TCP, buf, total_len);
    delete[] buf;
    return n;
}

ssize_t TCPConnection::send(const uint8_t *data, size_t len, uint16_t flags) {
    ssize_t n = send(data, len, flags, src_port_, dst_port_, seq_number_, ack_number_, src_ip_, dst_ip_);
    if (n > 0) {
        inc_seq_number(flags, len);
    }
    return n;
}

void TCPConnection::send(const uint8_t *data, size_t len) {
    send(data, len, get_flags({PSH_MASK, ACK_MASK}));
}

void TCPConnection::send() {
    std::unique_lock lck(write_m_);
    size_t size = _send_buf.size();
    if (size == 0) {
        return;
    }
    auto *tosend = new uint8_t[std::max(size, MSS)];
    if (size <= MSS) {
        std::copy(_send_buf.begin(), _send_buf.end(), tosend);
        send(tosend, size);
        _ack_buf.insert(_ack_buf.end(), _send_buf.begin(), _send_buf.end());
        _send_buf.clear();
    } else {
        std::copy(_send_buf.begin(), _send_buf.begin() + MSS, tosend);
        send(tosend, MSS);
        _ack_buf.insert(_ack_buf.end(), _send_buf.begin(), _send_buf.begin() + MSS);
        _send_buf.erase(_send_buf.begin(), _send_buf.begin() + MSS);
    }
    delete[] tosend;
}

void TCPConnection::recv(uint8_t const *data, size_t len, uint32_t from_ip) noexcept {
    auto *tcp = reinterpret_cast<tcp_t const *>(data);
    uint16_t flags = tcp->flags;
    uint16_t header_len = ((flags & 0x00f0) >> 4) * 4;
    if (header_len > len) {
        std::cerr << "malformed TCP segment. Size too small." << std::endl;
        return;
    }

    size_t payload_size = len - header_len;

    // step 1 If the state is CLOSED
    if (state_ == State::CLOSED) {
        send(nullptr, 0,
             has_ack(flags) ? get_flags({RST_MASK}) : get_flags({RST_MASK, ACK_MASK}),
             ntohs(tcp->dst_port),
             ntohs(tcp->src_port),
             has_ack(flags) ? tcp->ack : 0,
             has_ack(flags) ? 0 : nl_add_hl(tcp->seq, payload_size),
             src_ip_,
             from_ip
        );
        return;
    }

    // step 2   If the state is LISTEN then

    // step 3   If the state is SYN-SENT then
    if (state_ == State::SYN_SENT) {
        if (has_ack(flags)) {
            //  If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send a reset (unless
            //  the RST bit is set, if so drop the segment and return)
        }

        if (has_rst(flags)) {
            if (has_ack(flags)) {
                // signal the user "error: connection reset"
                state_ = State::CLOSED;
            }
            return;
        }

        if (has_syn(flags)) {
            ack_number_ = tcp->seq;
            inc_ack_number(1);
            send(nullptr, 0, get_flags({ACK_MASK}));
            state_ = State::ESTABLISHED;
            poll_flags_ |= (POLLOUT | POLLWRNORM | POLLWRBAND);
            if (established_cb_) {
                established_cb_(0);
            }
        }
        return;
    }

    // otherwise
    // first check sequence number
    if (has_rst(flags)) {
        switch (state_) {
            case State::ESTABLISHED:
            case State::FIN_WAIT_1:
            case State::FIN_WAIT_2:
            case State::CLOSE_WAIT:
//                If the RST bit is set then, any outstanding RECEIVEs and SEND
//                should receive "reset" responses.  All segment queues should be
//                flushed.  Users should also receive an unsolicited general
//                "connection reset" signal.  Enter the CLOSED state, delete the
//                TCB, and return.
                state_ = State::CLOSED;
                return;
            case State::CLOSING:
            case State::LAST_ACK:
            case State::TIME_WAIT:
                state_ = State::CLOSED;
                return;
        }
    }

    if (has_syn(flags)) {
        switch (state_) {
            case State::SYN_RECEIVED:
            case State::ESTABLISHED:
            case State::FIN_WAIT_1:
            case State::FIN_WAIT_2:
            case State::CLOSE_WAIT:
            case State::CLOSING:
            case State::LAST_ACK:
            case State::TIME_WAIT:
                inc_ack_number(1);
                send(nullptr, 0, get_flags({RST_MASK, ACK_MASK}));
                state_ = State::CLOSED;
                return;
        }
    }

    if (!has_ack(flags)) {
        return;
    }

//    if (has_ack(flags)) {
//        std::unique_lock lck(write_m_);
//        uint32_t acked = ntohl(tcp->ack);
//        if (acked > send_base_) {
//            if (acked - send_base_ >= _ack_buf.size()) {
//                _ack_buf.clear();
//            } else {
//                _ack_buf.erase(_ack_buf.begin(), _ack_buf.begin() + (acked - send_base_));
//            }
//            send_base_ = acked;
//        }
//        lck.unlock();
//    }

    if (payload_size > 0) {
        std::unique_lock<std::mutex> lck(_read_m);
        uint8_t const *payload = data + header_len;
        _recv_buf.insert(_recv_buf.end(), payload, payload + payload_size);
        poll_flags_ |= (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND);
        lck.unlock();
        inc_ack_number(payload_size);
        send(nullptr, 0, get_flags({ACK_MASK}));
    }

    if (has_fin(flags)) {
        inc_ack_number(1);
        send(nullptr, 0, get_flags({ACK_MASK}));

        switch (state_) {
            case State::SYN_RECEIVED:
            case State::ESTABLISHED:
                state_ = State::CLOSE_WAIT;
                break;
            case State::FIN_WAIT_1:
            case State::FIN_WAIT_2:
                state_ = State::TIME_WAIT;
                break;
            case State::CLOSE_WAIT:
            case State::CLOSING:
            case State::LAST_ACK:
            case State::TIME_WAIT:
                break;
        }
        poll_flags_ |= (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND);
    }
}

void TCPConnection::inc_seq_number(uint16_t flags, size_t payload_size) {
    if (has_syn(flags) || has_fin(flags)) {
        seq_number_ = nl_add_hl(seq_number_, 1);
    } else if (payload_size != 0) {
        seq_number_ = nl_add_hl(seq_number_, (uint32_t) payload_size);
    }
}

void TCPConnection::inc_ack_number(uint32_t inc) {
    ack_number_ = nl_add_hl(ack_number_, inc);
}

uint16_t TCPConnection::get_flags(std::initializer_list<uint16_t> masks, uint16_t header_len) {
    // max header length is 7 * 4 = 28
    uint16_t base = std::min((uint16_t) 28, header_len) / 4 * 16; // significant bit on uint8
    for (auto mask : masks) {
        base |= mask;
    }
    return base;
}

void TCPConnection::connect(uint32_t dst_ip, uint16_t dst_port) {
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<uint32_t> uniform_dist;

    seq_number_ = uniform_dist(e1);
    dst_ip_ = dst_ip;
    dst_port_ = dst_port;
    send(nullptr, 0, get_flags({SYN_MASK}));
    state_ = State::SYN_SENT;

//    // start send process
//    std::thread([&] {
//        while (true) {
//            this->send();
//            std::this_thread::sleep_for(std::chrono::milliseconds(100));
//        }
//    }).detach();
}

ssize_t TCPConnection::read(uint8_t *buf, size_t size) {
    std::unique_lock lck(_read_m);
//    std::cout << "before wait" << std::endl;
//    _cv.wait(lck, [&] { return _buffer_ready; });
//    std::cout << "after wait" << std::endl;
    if (_recv_buf.empty()) {
        return 0;
    }
    std::cout << "try to read" << size << "from server" << std::endl;
    std::cout << "Available size is " << _recv_buf.size() << std::endl;
    if (size > _recv_buf.size()) {
        size_t read_size = _recv_buf.size();
        std::copy(_recv_buf.begin(), _recv_buf.end(), buf);
        hexDump(nullptr, buf, read_size);
        _recv_buf.clear();
        if (state_ == State::ESTABLISHED) {
            _buffer_ready = false;
            poll_flags_ &= ~POLLIN;
        }
        return read_size;
    } else {
        std::copy(_recv_buf.begin(), _recv_buf.begin() + size, buf);
        _recv_buf.erase(_recv_buf.begin(), _recv_buf.begin() + size);
        hexDump(nullptr, buf, size);
        return size;
    }
}

ssize_t TCPConnection::write(uint8_t const *buf, size_t size) {
//    std::unique_lock lck(_write_m);
//    _send_buf.insert(_send_buf.end(), buf, buf + size);
//    printf("write %s\n", buf);
    send(buf, size);
    return size;
}

void TCPConnection::close() {
//    send(nullptr, 0, get_flags({FIN_MASK, ACK_MASK}));
//    state_ = State::CLOSED;
}
