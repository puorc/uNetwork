#include "Ethernet.h"

Ethernet::Ethernet(const NetworkDevice &device) : device(device) {
    _buf = new uint8_t[device.mtu];
}

Ethernet::~Ethernet() {
    delete[] _buf;
}

ssize_t Ethernet::send(const HwAddr &dmac, Ethernet::protocol type, uint8_t *data, size_t len) {
    size_t buf_size = sizeof(struct ethernet_t) + len;
    auto *buf = new uint8_t[buf_size];
    uint8_t *ptr = buf;

    auto *eth = reinterpret_cast<ethernet_t *>(ptr);

    eth->ethertype = htons(static_cast<uint16_t>(type));
    std::copy(device.mac_addr, device.mac_addr + 6, eth->smac);
    std::copy(std::begin(dmac), std::end(dmac), eth->dmac);
    ptr += sizeof(struct ethernet_t);
    std::copy(data, data + len, ptr);

    ssize_t n = device.tun_write(buf, buf_size);
    delete[] buf;
    return n;
}

Result Ethernet::recv() {
    ssize_t n;
    if ((n = device.tun_read(_buf, device.mtu)) <= 0) {
        return Result{.data=nullptr, .protocol=0, .size=0};
    }
    auto *eth = reinterpret_cast<ethernet_t *>(_buf);
    return {.data=_buf + sizeof(ethernet_t), .protocol=ntohs(eth->ethertype), .size=n - sizeof(struct ethernet_t)};
}
