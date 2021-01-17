#include <cstring>
#include "TCPConnection.h"
#include "ipv4.h"

const uint32_t network_order_plus_one = 0x1000000;

constexpr uint16_t get_flags(int header_len, uint16_t mask) {
    return (uint16_t) (16 * header_len / 4) | mask;
}

void TCPConnection::send(uint8_t *data, size_t len, uint16_t flags) {
    size_t total_len = len + sizeof(struct tcp_t);
    uint8_t *buf = new uint8_t[total_len];
    uint8_t *ptr = buf;

    struct tcp_t tcp;
    tcp.src_port = src_port;
    tcp.dst_port = dst_port;
    tcp.seq = seq_number;
    tcp.ack = ack_number;
    tcp.flags = flags;
    tcp.window = 0xbdad;
    tcp.checksum = 0;
    tcp.urgent_pt = 0;

    memcpy(ptr, &tcp, sizeof(tcp_t));
    ptr += sizeof(tcp_t);
    memcpy(ptr, data, len);

    struct tcp_t *ss = reinterpret_cast<tcp_t *>(buf);
    ss->checksum = tcp_udp_checksum(src_ip, dst_ip, IP_TCP, buf, total_len);

    ipv4_send(src_ip, dst_ip, IP_TCP, buf, total_len);
    delete[] buf;
}

void TCPConnection::init() {
    send(nullptr, 0, get_flags(20, SYN_MASK));
    state = State::SYN_SEND;
}

void TCPConnection::send(uint8_t *data, size_t len) {
    send(data, len, 0x1850);
}

void TCPConnection::receive(uint8_t *data, size_t len) {
    struct tcp_t *tcp = (struct tcp_t *) data;
    uint16_t flags = tcp->flags;
    uint16_t header_len = ((flags & 0xf000) >> 12);

    bool is_ack = ((flags & ACK_MASK) != 0);
    bool is_syn = ((flags & SYN_MASK) != 0);
    bool is_fin = ((flags & FIN_MASK) != 0);

    if (is_ack && is_syn) {
        ack_number = tcp->seq + network_order_plus_one;
        seq_number += network_order_plus_one;
        send(nullptr, 0, get_flags(20, ACK_MASK));
        state = State::ESTABLISHED;
    } else if (is_ack) {
        std::cout << "get ack" << std::endl;
        std::cout.flush();
    } else if (is_fin) {
        std::cout << "get fin" << std::endl;
        std::cout.flush();
    } else {
        std::cout << "data" << std::endl;
    }
}


