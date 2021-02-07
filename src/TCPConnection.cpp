#include <thread>
#include "TCPConnection.h"

ssize_t TCPConnection::send(const uint8_t *data, size_t len, uint16_t flags) {
    size_t total_len = len + sizeof(struct tcp_t);
    auto *buf = new uint8_t[total_len];

    auto *tcp = reinterpret_cast<tcp_t *>(buf);
    tcp->src_port = _src_port;
    tcp->dst_port = _dst_port;
    tcp->seq = seq_number;
    tcp->ack = ack_number;
    tcp->flags = flags;
    tcp->window = 0xbdad;
    tcp->checksum = 0;
    tcp->urgent_pt = 0;

    std::copy(data, data + len, buf + sizeof(struct tcp_t));
    tcp->checksum = tcp_udp_checksum(_src_ip, _dst_ip, IP_TCP, buf, total_len);

    ssize_t n = ip.send(_dst_ip, IP_TCP, buf, total_len);
    if (n > 0) {
        inc_seq_number(flags, len);
    }
    delete[] buf;
    return n;
}

void TCPConnection::send(const uint8_t *data, size_t len) {
    send(data, len, get_flags({PSH_MASK, ACK_MASK}));
}

void TCPConnection::send() {
    std::unique_lock lck(_write_m);
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

void TCPConnection::recv(uint8_t const *data, size_t len) noexcept {
    auto *tcp = reinterpret_cast<tcp_t const *>(data);
    uint16_t flags = tcp->flags;
    uint16_t header_len = ((flags & 0x00f0) >> 4) * 4;
    if (header_len > len) {
        std::cerr << "malformed TCP segment. Size too small." << std::endl;
        return;
    }
    size_t payload_size = len - header_len;

    if (is_ack(flags)) {
        std::unique_lock lck(_write_m);
        uint32_t acked = ntohl(tcp->ack);
        if (acked > _send_base) {
            if (acked - _send_base >= _ack_buf.size()) {
                _ack_buf.clear();
            } else {
                _ack_buf.erase(_ack_buf.begin(), _ack_buf.begin() + (acked - _send_base));
            }
            _send_base = acked;
        }
        lck.unlock();
    }
    if (is_syn(flags)) {
        ack_number = tcp->seq;
        inc_ack_number(flags, payload_size);
        send(nullptr, 0, get_flags({ACK_MASK}));
        state = State::ESTABLISHED;
        poll_flags |= (POLLOUT | POLLWRNORM | POLLWRBAND);
        if (_established_cb) {
            _established_cb(0);
        }
        return;
    }
    if (is_fin(flags) || payload_size > 0) {
        {
            std::unique_lock<std::mutex> lck(_read_m);
            if (payload_size > 0) {
                uint8_t const *payload = data + header_len;
                _recv_buf.insert(_recv_buf.end(), payload, payload + payload_size);
                poll_flags |= (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND);
            }
            _buffer_ready = true;
            lck.unlock();
//            _cv.notify_one();
        }
        if (is_fin(flags)) {
            poll_flags |= (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND);
            state = State::CLOSE_WAIT;
        }

        inc_ack_number(flags, payload_size);
        send(nullptr, 0, get_flags({ACK_MASK}));
    }
}

void TCPConnection::inc_seq_number(uint16_t flags, size_t payload_size) {
    if (is_syn(flags) || is_fin(flags)) {
        seq_number = nl_add_hl(seq_number, 1);
    } else if (payload_size != 0) {
        seq_number = nl_add_hl(seq_number, (uint32_t) payload_size);
    }
}

void TCPConnection::inc_ack_number(uint16_t flags, size_t payload_size) {
    if (is_syn(flags)) {
        ack_number = nl_add_hl(ack_number, 1);
        return;
    }
    uint32_t total = 0;
    if (is_fin(flags)) {
        ++total;
    }
    if (payload_size != 0) {
        total += payload_size;
    }
    ack_number = nl_add_hl(ack_number, total);
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

    seq_number = uniform_dist(e1);
    _dst_ip = dst_ip;
    _dst_port = dst_port;
    send(nullptr, 0, get_flags({SYN_MASK}));
    state = State::SYN_SEND;

//    // start send process
//    std::thread([&] {
//        while (true) {
//            this->send();
//            std::this_thread::sleep_for(std::chrono::milliseconds(100));
//        }
//    }).detach();
}

void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *) addr;

    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
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
        if (state == State::ESTABLISHED) {
            _buffer_ready = false;
            poll_flags &= ~POLLIN;
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
    state = State::CLOSED;
}
