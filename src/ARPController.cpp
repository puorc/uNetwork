#include "ARPController.h"

void ARPController::recv(uint8_t *data, size_t size) {
    if (size < sizeof(struct arp_t)) {
        return;
    }
    auto *arp = reinterpret_cast<arp_t *>(data);
    if (static_cast<Opcode>(arp->opcode) == Opcode::Reply) {
        Ethernet::HwAddr addr;
        std::copy(arp->sender_hw_addr, arp->sender_hw_addr + 6, std::begin(addr));
        table[arp->sender_protocol_addr] = std::make_pair(addr, time(nullptr));
    } else if (static_cast<Opcode>(arp->opcode) == Opcode::Request) {
        if (arp->target_protocol_addr == dev.ip_addr) {
            reply(data, size);
        }
    }
}

void ARPController::reply(uint8_t *data, size_t size) {
    auto *arp = reinterpret_cast<arp_t *>(data);
    auto *resp = new arp_t();

    resp->opcode = static_cast<uint16_t>(Opcode::Reply);
    resp->hw_addr_len = arp->hw_addr_len;
    resp->hw_type = arp->hw_type;
    resp->protocol_addr_len = arp->protocol_addr_len;
    resp->protocol_type = arp->protocol_type;

    resp->target_protocol_addr = arp->sender_protocol_addr;
    resp->sender_protocol_addr = dev.ip_addr;
    std::copy(arp->sender_hw_addr, arp->sender_hw_addr + 6, resp->target_hw_addr);
    std::copy(dev.mac_addr, dev.mac_addr + 6, resp->sender_hw_addr);

    Ethernet::HwAddr addr;
    std::copy(dev.mac_addr, dev.mac_addr + 6, std::begin(addr));

    ssize_t n = eth.send(addr, Ethernet::protocol::ARP, reinterpret_cast<uint8_t *>(resp), sizeof(struct arp_t));
    if (n <= 0) {
        std::cerr << "ARP reply failure" << std::endl;
        std::cerr.flush();
    }
    delete resp;
}

void ARPController::send(uint32_t ip_addr) {
    auto *req = new arp_t();

    req->opcode = static_cast<uint16_t>(Opcode::Request);
    req->hw_addr_len = 6;
    req->hw_type = htons(1);
    req->protocol_addr_len = 4;
    req->protocol_type = htons(0x0800);

    req->target_protocol_addr = ip_addr;
    req->sender_protocol_addr = dev.ip_addr;

    std::fill(std::begin(req->target_hw_addr), std::end(req->target_hw_addr), 0xff);
    std::copy(dev.mac_addr, dev.mac_addr + 6, req->sender_hw_addr);

    ssize_t n = eth.send({0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, Ethernet::protocol::ARP,
                         reinterpret_cast<uint8_t *>(req),
                         sizeof(struct arp_t));
    if (n <= 0) {
        std::cerr << "ARP request failure" << std::endl;
        std::cerr.flush();
    }
    delete req;
}

Ethernet::HwAddr ARPController::query(uint32_t ip_addr) {
    if (table.find(ip_addr) != table.end()) {
        return table[ip_addr].first;
    } else {
        return {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    }
}

ARPController::ARPController(Ethernet &eth, NetworkDevice &dev) : eth(eth), dev(dev) {}
