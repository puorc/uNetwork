#include "ethernet.h"

const int MAX_PACKET_SIZE = 8192;

ssize_t ethernet_recv(uint8_t *buf, ethernet_protocol *protocol) {
    uint8_t data[MAX_PACKET_SIZE];
    ssize_t n;
    if ((n = tun_read(data, MAX_PACKET_SIZE)) < 0) {
        return n;
    }
    if (n < sizeof(struct ethernet_t)) {
        // incomplete data
        return -1;
    }

    struct ethernet_t *eth = reinterpret_cast<ethernet_t *>(data);
    uint8_t *payload = data + sizeof(struct ethernet_t);
    size_t data_size = n - sizeof(struct ethernet_t);
    std::copy(payload, payload + data_size, buf);
    *protocol = static_cast<ethernet_protocol>(ntohs(eth->ethertype));
    return data_size;
}

ssize_t ethernet_send(uint8_t *smac, uint8_t *dmac, ethernet_protocol type, uint8_t *data, size_t len) {
    size_t buf_size = sizeof(struct ethernet_t) + len;
    auto *buf = new uint8_t[buf_size];
    uint8_t *ptr = buf;

    struct ethernet_t *eth = reinterpret_cast<ethernet_t *>(ptr);
    eth->ethertype = htons(static_cast<uint16_t>(type));
    memcpy(eth->dmac, dmac, 6);
    memcpy(eth->smac, smac, 6);
    ptr += sizeof(struct ethernet_t);
    memcpy(ptr, data, len);

    ssize_t n = tun_write(buf, buf_size);
    delete[] buf;
    return n;
}

ssize_t ethernet_send_with_mac(char const *smac, char const *dmac, ethernet_protocol type, uint8_t *data, size_t len) {
    auto *smac_arr = new uint8_t[6];
    auto *dmac_arr = new uint8_t[6];

    sscanf(smac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &smac_arr[0],
           &smac_arr[1],
           &smac_arr[2],
           &smac_arr[3],
           &smac_arr[4],
           &smac_arr[5]);

    sscanf(dmac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dmac_arr[0],
           &dmac_arr[1],
           &dmac_arr[2],
           &dmac_arr[3],
           &dmac_arr[4],
           &dmac_arr[5]);
    size_t sent = ethernet_send(smac_arr, dmac_arr, type, data, len);

    delete[] smac_arr;
    delete[] dmac_arr;

    return sent;
}
