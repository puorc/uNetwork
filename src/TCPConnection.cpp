#include "TCPConnection.h"

void TCPConnection::send(uint8_t *data, size_t len, uint16_t flags) {
    size_t total_len = len + sizeof(struct tcp_t);
    auto *buf = new uint8_t[total_len];

    auto *tcp = reinterpret_cast<tcp_t *>(buf);
    tcp->src_port = src_port;
    tcp->dst_port = dst_port;
    tcp->seq = seq_number;
    tcp->ack = ack_number;
    tcp->flags = flags;
    tcp->window = 0xbdad;
    tcp->checksum = 0;
    tcp->urgent_pt = 0;

    memcpy(buf + sizeof(struct tcp_t), data, len);
    tcp->checksum = tcp_udp_checksum(src_ip, dst_ip, IP_TCP, buf, total_len);

    ssize_t n;
    if ((n = ip.send(dst_ip, IP_TCP, buf, total_len)) < 0) {
        std::cout << "send failure. retransmit scheduled";
        std::cout.flush();
        delete[] buf;
        return;
    }

//    if ((n = ipv4_send(src_ip, dst_ip, IP_TCP, buf, total_len)) < 0) {
//        std::cout << "send failure. retransmit scheduled";
//        std::cout.flush();
//        delete[] buf;
//        return;
//    }
    delete[] buf;
    inc_seq_number(flags, len);
}

void TCPConnection::init() {
    send(nullptr, 0, get_flags({SYN_MASK}));
    state = State::SYN_SEND;
}

void TCPConnection::send(uint8_t *data, size_t len) {
    send(data, len, get_flags({PSH_MASK, ACK_MASK}));
}

void TCPConnection::receive(uint8_t const *data, size_t len) {
    auto *tcp = reinterpret_cast<tcp_t const *>(data);
    uint16_t flags = tcp->flags;
    uint16_t header_len = ((flags & 0x00f0) >> 4) * 4;
    if (header_len > len) {
        std::cerr << "malformed TCP segment. Size too small";
        std::cerr.flush();
        return;
    }
    size_t payload_size = len - header_len;

    if (is_ack(flags)) {
        send_base = std::max(ntohl(tcp->ack), send_base);
    }
    if (is_syn(flags)) {
        ack_number = tcp->seq;
        inc_ack_number(flags, payload_size);
        send(nullptr, 0, get_flags({ACK_MASK}));
        state = State::ESTABLISHED;
        return;
    } else if (is_fin(flags)) {
        inc_ack_number(flags, payload_size);
        state = State::FIN_WAIT_1;
        send(nullptr, 0, get_flags({ACK_MASK, FIN_MASK}));
    } else if (payload_size > 0) {
        uint8_t const *payload = data + header_len;
        std::cout << "payload is" << std::endl << payload << std::endl;
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
    if (is_syn(flags) || is_fin(flags)) {
        ack_number = nl_add_hl(ack_number, 1);
    } else if (payload_size != 0) {
        ack_number = nl_add_hl(ack_number, (uint32_t) payload_size);
    }
}

uint16_t TCPConnection::get_flags(std::initializer_list<uint16_t> masks, uint16_t header_len) {
    // max header length is 7 * 4 = 28
    uint16_t base = std::min((uint16_t) 28, header_len) / 4 * 16; // significant bit on uint8
    for (auto mask : masks) {
        base |= mask;
    }
    return base;
}


