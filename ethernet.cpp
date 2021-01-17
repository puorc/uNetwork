#include "ethernet.h"

int receive_ethernet(uint8_t *data, int in_len, uint8_t **ptr, int *len) {
    struct ethernet_t *eth = (struct ethernet_t *) data;
    *ptr = data + sizeof(struct ethernet_t);
    *len = in_len - sizeof(struct ethernet_t);
    return be16toh(eth->ethertype);
}

void send_ethernet(uint8_t *smac, uint8_t *dmac, uint16_t type, uint8_t *data, int len) {
    uint8_t *packet = static_cast<uint8_t *>(malloc(sizeof(struct ethernet_t) + len));
    uint8_t *ptr = packet;

    struct ethernet_t eth;
    eth.ethertype = htobe16(type);
    memcpy(eth.dmac, dmac, 6);
    memcpy(eth.smac, smac, 6);
    memcpy(ptr, &eth, sizeof(struct ethernet_t));
    ptr += sizeof(struct ethernet_t);
    memcpy(ptr, data, len);

    tun_write(packet, sizeof(struct ethernet_t) + len);
}

void send_ethernet_str(char *smac, char *dmac, uint16_t type, uint8_t *data, int len) {
    uint8_t *smac_arr = new uint8_t[6];
    uint8_t *dmac_arr = new uint8_t[6];

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
    send_ethernet(smac_arr, dmac_arr, type, data, len);
}
